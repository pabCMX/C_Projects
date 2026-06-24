#ifndef _WIN32
#define _DEFAULT_SOURCE
#include <stddef.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

__extension__ typedef unsigned __int128 u128;

uint64_t isqrt(uint64_t n) {
  if (n == 0)
    return 0;

  uint64_t lo = 1;
  uint64_t hi = n;

  while (lo < hi) {
    uint64_t mid = lo + (hi - lo + 1) / 2;
    if (mid <= n / mid) {
      lo = mid;
    } else {
      hi = mid - 1;
    }
  }
  return lo;
}

static void print_u128(FILE *f, u128 v) {
  if (v == 0) {
    fputc('0', f);
    return;
  }

  char   buffer[40]; // Enough to fit a u128 long number
  size_t n = 0;

  while (v > 0) {
    // A little terse but in short, for a given buffer slot, we take the last digit of the num, and
    // cast it to a char. Since chararacters are treated as ints, we can 'add' to get the right
    // digit character in binary.
    buffer[n++] = (char)('0' + (v % 10));
    // Then we divide by 10, truncating the number by one decimal place.
    v /= 10;
  }

  while (n > 0) {
    // Now in reverse, we build the character string from the front backwards directly in the text
    // stream.
    fputc(buffer[--n], f);
  }
}

// Parse and validates endpoint argument to a number.
static int parse_args(const char *text, uint64_t *end) {
  char *endptr;
  errno = 0;
  *end  = strtoull(text, &endptr, 10);
  if (endptr == text || *endptr != '\0' || *end < 2) {
    printf("Invalid endpoint: %s\n", text);
    return -1;
  } else if (errno == ERANGE) {
    printf("Endpoint too large, use a smaller number. Exiting...");
    return -1;
  }
  return 0;
}

// Outputs terminal updates for run progress.
static void run_terminal_updates(uint64_t high, uint64_t end, uint32_t *last_update,
                                 uint64_t *total_primes_counter) {
  // Update handling
  uint32_t updates        = 2000;
  uint32_t current_update = high * updates / end;
  if (current_update > *last_update && isatty(STDERR_FILENO)) {

    fprintf(stderr, "\rPrime count: %-10lu | Completion %%: %5.2f", *total_primes_counter,
            100.0 * current_update / updates);
    fflush(stderr);
    *last_update = current_update;
  }
}

// Run initial prime search from 2 to sqrt(end), and return total primes counted. Bool Sieve instead
// of Find and Store.
static uint32_t *pre_sieve(uint64_t end, uint64_t *base_prime_count) {

  uint32_t count = 0; // making sure we're set to zero.
  uint64_t root  = isqrt(end);
  // Setup the bool array with all zeros
  unsigned char *is_composite = calloc(root + 1, 1);
  // Handle errors
  if (is_composite == NULL) {
    printf("Unable to allocate presieve composites array. Exiting...\n");
    return NULL;
  }

  // Set the obvious 0 and 1 as composite
  is_composite[0] = 1;
  is_composite[1] = 1;

  // Start a loop from 2 onwards
  for (uint32_t p = 2; p <= root; p++) {
    if (!is_composite[p]) {
      uint64_t p_squared = p * p;

      if (p_squared > root) {
        // We've found the last prime that still strikes within the pre-sieve.
        break;
      }
      for (uint32_t j = p_squared; j <= root; j += p) {
        is_composite[j] = 1;
      }
    }
  }

  // Now we count the primes we have left.
  for (uint32_t i = 0; i <= root; i++) {
    if (!is_composite[i]) {
      count++;
    }
  }

  // Then allocate the base_primes array.
  uint32_t *primes = malloc(count * sizeof(uint32_t));
  // And fill it.
  uint32_t index = 0;
  for (uint32_t i = 0; i <= root; i++) {
    if (!is_composite[i]) {
      primes[index++] = i;
    }
  }

  // We finished the presieve setup, return and start the complete bool sieve.
  free(is_composite);
  *base_prime_count = count;
  return primes;
}

// Finds all primes below given end with given composite array.
static int odds_seg_sieve(uint32_t *primes, uint64_t base_prime_count, unsigned char *is_composite,
                          uint64_t *next_cursor, uint64_t high_num, uint64_t low_num,
                          u128 *out_p_sum, uint64_t *out_p_count) {

  u128     segment_sum   = 0;
  uint64_t segment_count = 0;
  uint64_t segment_size  = ((high_num - low_num + 1) / 2);

  // If it's the first block, we set the obvious ones to composite:
  if (low_num == 1) {
    is_composite[0] = 1;
  }

  // Then we iterate through every prime in primes[] that has a multiple within the segment.
  for (uint64_t i = 0; i < base_prime_count; i++) {
    uint64_t p         = primes[i];
    uint64_t p_squared = p * p;
    if (p == 2)
      continue;
    if (p_squared >= high_num)
      break;
    // We need to compute the segment relative offset for the otherwise global index in next_cursor
    uint64_t segment_base = (low_num - 1) / 2;
    uint64_t offset       = next_cursor[i] - segment_base;

    // Now while we are below the size of the segment, we stride by p and mark each multiple.
    while (offset < segment_size) {
      is_composite[offset] = 1;
      offset += p;
    }

    // When we're done, we save the next multiple, adding back segment_base to get the global index
    next_cursor[i] = offset + segment_base;
  }
  // Accumulate sum and prime count from the composite sieve.
  for (uint64_t i = 0; i < segment_size; i++) {
    if (!is_composite[i]) {
      segment_sum += low_num + (2 * i);
      segment_count++;
    }
  }
  *out_p_count += segment_count;
  *out_p_sum += segment_sum;
  return 0;
}

// Sets up and manages the main sieve loop.
static int run_full_search(uint32_t *primes, uint64_t base_prime_count, uint32_t block_size,
                           uint64_t end, u128 *prime_sum, uint64_t *total_primes_counter,
                           _Bool sum_only) {
  uint32_t       last_update  = 0;
  unsigned char *is_composite = calloc(block_size, 1);

  // We initialize the rolling cursor array to eliminate a ton of our warm loop math.
  uint64_t *next_cursor = malloc(base_prime_count * sizeof(uint64_t));
  for (uint32_t i = 0; i < base_prime_count; i++) {
    // Starting with p squared, as the first possible multiple.
    uint64_t p         = primes[i];
    uint64_t p_squared = p * p;

    // We map that square to the odds-only index
    next_cursor[i] = (p_squared - 1) / 2;
  }
  if (is_composite == NULL || next_cursor == NULL) {
    printf("Unable to allocate presieve composites array. Exiting...\n");
    return -1;
  }
  // Set up a loop to iterate by segment[i]
  for (uint32_t i = 0;; i++) {

    // Set the high and low numbers for the segment.
    uint64_t low  = ((i * (uint64_t)block_size) * 2) + 1;
    uint64_t high = (((i + 1) * (uint64_t)block_size) * 2) + 1;

    if (low >= end) {
      // If the segment low number jumps over the end, we're done.
      break;
    }
    if (high > end) {
      high = end;
    }

    if (odds_seg_sieve(primes, base_prime_count, is_composite, next_cursor, high, low, prime_sum,
                       total_primes_counter) != 0)
      return -1;
    // Reset for next cycle
    memset(is_composite, 0, block_size);
    if (!sum_only)
      run_terminal_updates(high, end, &last_update, total_primes_counter);
  }
  free(next_cursor);
  free(is_composite);

  return 0;
}

// Segmented Odds-only E-sieve with rolling cursors. Searches for primes between 2 and given arg or
// 2 and 2^31 with no arg. Hoping for ~50% better perf than non-odds only version on ends ~2^31.
int main(int argc, char *argv[]) {
  const char *endpoint_arg         = NULL;
  _Bool       sum_only             = false;
  uint64_t    end                  = 1;
  u128        prime_sum            = 2;       // Adding 2 to start.
  uint64_t    total_primes_counter = 1;       // Counting 2
  uint64_t    primes_capacity      = 64;      // Base primes array size.
  uint64_t    base_prime_count     = 0;       // Primes in sqrt(end) base sieve array.
  uint64_t    block_size           = 1 << 19; // Segment size (~512KiB of unsigned char).
  if (argc > 3) {
    // If we get too many arguments, exit with error
    printf("Usage: %s <endpoint> [--sum-only]\n", argv[0]);
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--sum-only") == 0) {
      sum_only = true;
    } else if (endpoint_arg == NULL) {
      endpoint_arg = argv[i];
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  if (endpoint_arg == NULL) {
    // If we didn't get an endpoint just use 2^31 by performing 31 bit shifts on 1.
    // Idiomatic I think.
    end = 1ull << 31;
    if (!sum_only)
      printf("No arguments found\n");
  } else {
    // If we got an argument to sum to,
    // we convert from text to an unsigned 64-bit int with strtoull()
    if (parse_args(endpoint_arg, &end) != 0)
      return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Set endpoint to %lu\n", end);
    printf("Starting Search...\n");
  }

  if (primes_capacity < isqrt(end) / 2 + 16)
    primes_capacity = isqrt(end) / 2 + 16; // Lazy fudge for presize.
  // Late assign/alloc so we don't have to free on each possible error before now.

  // Now run the pre_sieve for the primes < sqrt(end)
  uint32_t *base_primes = pre_sieve(end, &base_prime_count);
  if (base_primes == NULL) {
    printf("Error allocating primes array.\n");
    free(base_primes);
    return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Presieve complete.\n");
    printf("Base Prime Count is %lu\n", base_prime_count);
    printf("Starting segmented sieve...\n");
  }

  // If we didn't have any issues, run the real segmented sieve.
  if (run_full_search(base_primes, base_prime_count, block_size, end, &prime_sum,
                      &total_primes_counter, sum_only) != 0) {
    free(base_primes);
    return EXIT_FAILURE;
  }

  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\rPrime count: %-10lu Completion %%: %3.2f \n", total_primes_counter, 100.00f);
    fflush(stderr);
  }

  if (!sum_only) {
    printf("Found %lu primes [0, %lu)\n", total_primes_counter, end);
    fputs("Sum of found primes is ", stdout);
    print_u128(stdout, prime_sum);
    putchar('\n');
  } else {
    print_u128(stdout, prime_sum);
    putchar('\n');
  }

  free(base_primes);
  return 0;
}
