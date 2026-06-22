#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

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

static int parse_args(const char *text, uint64_t *end, uint64_t *primes_capacity) {
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
  if (*primes_capacity < isqrt(*end) / 2 + 16)
    *primes_capacity = isqrt(*end) / 2 + 16; // Lazy fudge for presize.
  return 0;
}
/*
//Resize array function for possible prime array fudge being wrong.
static int resize_array(uint64_t **array, uint64_t *capacity) {
  // Otherwise we expand by 2x primes_capacity.
  uint64_t new_capacity = *capacity * 2ul;

  // Create a temporary array *pointer* to a bigger array.
  uint64_t *tmp = realloc(*array, new_capacity * sizeof(**array));

  // Handle errors.
  if (tmp == NULL) {
    free(*array);
    printf("Unable to reallocate primes array. Exiting...\n");
    return EXIT_FAILURE;
  }

  // Finally set primes pointer to new array, and primes_capacity to new_capacity.
  *array    = tmp;
  *capacity = new_capacity;

  return 0;
}
*/

// Run initial prime search from 2 to sqrt(end), and return total primes counted. Bool Sieve instead
// of Find and Store.
static int pre_sieve(uint32_t *primes, uint64_t end, uint64_t *base_prime_count) {

  *base_prime_count = 0; // making sure we're set to zero.
  uint32_t root     = isqrt(end);
  // Setup the bool array
  unsigned char *is_composite = malloc(root);
  // Handle errors
  if (is_composite == NULL) {
    printf("Unable to allocate presieve composites array. Exiting...\n");
    return -1;
  }

  // Set everything to zeros
  memset(is_composite, 0, root + 1);

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
      for (uint32_t j = p_squared; j < root; j += p) {
        is_composite[j] = 1;
      }
    }
  }

  // Now we save the primes we have left while also updating our counter;

  for (uint32_t i = 0; i <= root; i++) {
    if (!is_composite[i]) {
      primes[(*base_prime_count)++] = i;
    }
  }
  // We finished the presieve setup, return and start the complete bool sieve.
  free(is_composite);
  return 0;
}

// Sets given segmented sieve
static void seg_sieve(uint32_t *primes, unsigned char *is_composite, uint64_t base_prime_count,
                      uint64_t low_index, uint64_t high_index, __int128 *out_p_sum,
                      uint64_t *out_p_count) {

  // If it's the first block, we set the obvious ones to composite:
  if (low_index < 1) {
    is_composite[0] = 1;
    is_composite[1] = 1;
  }
  __int128 segment_sum   = 0;
  uint64_t segment_count = 0;
  uint64_t segment_size  = high_index - low_index;

  // Then we iterate through every prime in primes[] that has a multiple within the segment.
  for (uint64_t i = 0; i < base_prime_count; i++) {
    uint64_t p         = primes[i];
    uint64_t p_squared = p * p;

    // Start with first multiple of p that is >= low
    uint64_t start = low_index + ((p - (low_index % p)) % p);

    // If p^2 is greater than the multiple, just use that.
    if (start < p_squared) {
      start = p_squared;
    }

    // If we're beyond the high bound, just break here.
    if (start >= high_index) {
      break;
    }

    // Finally calculate the offset from our multiple to the segment index.
    // and check off multiples until we hit the end of the sieve.
    for (uint64_t offset = start - low_index; offset < segment_size; offset += p) {
      is_composite[offset] = 1;
    }
  }

  // Accumulate sum and prime count from the composite sieve.
  for (uint64_t i = 0; i < segment_size; i++) {
    if (!is_composite[i]) {
      segment_sum += low_index + i;
      segment_count++;
    }
  }
  *out_p_count += segment_count;
  *out_p_sum += segment_sum;
}

// Square Root of end prime search w/ segmented odds - only sieve to find sum of all primes
// between 2 and 2^31 or given arg. ~5x better than non-odds only version on ends
// greater than ~2^23.
int main(int argc, char *argv[]) {
  const char  *endpoint_arg         = NULL;
  _Bool        sum_only             = false;
  uint64_t     end                  = 1;
  __int128     prime_sum            = 0;
  uint64_t     total_primes_counter = 0;
  uint64_t     primes_capacity      = 64;      // Base primes array size.
  uint64_t     base_prime_count     = 0;       // Primes in sqrt(end) base sieve array.
  uint64_t     block_size           = 1 << 20; // Segment size (~1MB of unsigned char).
  unsigned int updates              = 1000;    // Status Updates per run.
  unsigned int current_update       = 0;       // current status count
  unsigned int last_update          = 0;       // Last status update count.
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
    // we convert from text to an __int128 int with strtoull()
    if (parse_args(endpoint_arg, &end, &primes_capacity) != 0)
      return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Set endpoint to %lu\n", end);
    printf("Starting Search...\n");
  }

  // Late assignment so we don't have to free on each possible error before now.
  uint32_t *primes = malloc(primes_capacity * sizeof(*primes));

  // Handle errors
  if (primes == NULL) {
    printf("Error allocating primes array.\n");
    return EXIT_FAILURE;
  }

  // Now run the pre_sieve for the primes < sqrt(end)

  if (pre_sieve(primes, end, &base_prime_count) != 0) {
    free(primes);
    return EXIT_FAILURE;
  }
  if (!sum_only) {
    printf("Presieve complete.\n");
    printf("Base Prime Count is %lu\n", base_prime_count);
    printf("Starting segmented sieve...\n");
    printf("%lu segments needed...\n", (end + (block_size - 1)) / block_size);
  }

  // If we didn't have any issues, run the real sieve.
  // First allocate the single bool array buffer.
  unsigned char *is_composite = malloc(block_size * sizeof(unsigned char));
  // Handle errors.
  if (is_composite == NULL) {
    printf("Error allocating flag sieve.\n");
    free(primes);
    return EXIT_FAILURE;
  }

  // Then set up a loop to run the sieves sequentially, and accumulate the intermediate sum &
  // count.
  for (uint64_t i = 0;; i++) {

    // Set the high and low indexes.
    uint64_t low  = i * block_size;
    uint64_t high = (i + 1) * block_size;
    // Clear the bool array to zero (this also only touches the necessary bytes)
    memset(is_composite, 0, high - low);

    if (low >= end) {
      // If the segment is jumps over the end, we're done.
      break;
    }
    if (high > end) {
      high = end;
    }
    seg_sieve(primes, is_composite, base_prime_count, low, high, &prime_sum, &total_primes_counter);

    // Update handling
    current_update = high * updates / end;

    if (!sum_only && current_update > last_update && isatty(STDERR_FILENO)) {

      fprintf(stderr, "\rPrime count: %-10lu | Sum: %20llu | Completion %%: %5.2f",
              total_primes_counter, prime_sum, 100.0 * current_update / updates);
      fflush(stderr);
      last_update = current_update;
    }
  }

  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\rPrime count: %-10lu | Sum: %20llu | Completion %%: %3.2f \n",
            total_primes_counter, prime_sum, 100.00f);
    fflush(stderr);
  }

  if (!sum_only) {
    printf("Found %lu primes [0, %lu)\n", total_primes_counter, end);
    printf("Sum of found primes is %llu\n", prime_sum);
  } else {
    printf("%llu\n", prime_sum);
  }

  free(primes);
  free(is_composite);
  return 0;
}
