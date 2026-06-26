#ifndef _WIN32
#define _DEFAULT_SOURCE
#include <stdint.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static int resize_array(uint32_t **array, uint64_t *capacity) {
  // Otherwise we expand by 2x primes_capacity.
  uint64_t new_capacity = *capacity * 2ul;

  // Create a temporary array *pointer* to a bigger array.
  uint32_t *tmp = realloc(*array, new_capacity * sizeof(**array));

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

// Run initial prime search from 2 to sqrt(end), and return total primes counted.
static int pre_sieve(uint32_t **primes, uint64_t *primes_capacity, uint64_t *base_prime_count,
                     uint64_t start, uint64_t end) {

  _Bool composite = false;
  // Set the obvious 2 as prime.
  (*primes)[0]      = 2u;
  *base_prime_count = 1u;

  for (uint32_t i = start;; i++) {
    // Skip evens
    if (i % 2 == 0) {
      continue;
    }

    uint64_t i_squared = (uint64_t)i * i;
    // If i squared passes the end, we've checked past the root of end and can stop.
    if (i_squared > end)
      break;

    // Reset flag.
    composite = false;
    // We setup a loop, and make sure to skip 2 since we already know i isn't even.
    // 3 should not run at all.
    for (uint32_t j = 1; j < *base_prime_count; j++) {
      uint64_t p         = (*primes)[j];
      uint64_t p_squared = p * p;

      // If our prime^2 is bigger than i, no sense checking it or any other prime.
      if (p_squared > i)
        break;
      // Check i against our primes list so far.
      if (i % p == 0) {
        // If divisible just break and set the 'composite' flag.
        composite = true;
        break;
      }
    }

    // If we made it out of the loop before without composite getting set, we found a prime.
    if (!composite) {
      // Now we check if *primes[] is big enough to fit our new number. Just in case.
      if (*base_prime_count >= *primes_capacity) {
        // Otherwise we resize this array.
        if (resize_array(primes, primes_capacity) != 0)
          return -1;
      }

      // Set the latest prime
      (*primes)[*base_prime_count] = i;
      // And update our counters for how many primes we've found.
      (*base_prime_count)++;
    }
  }
  // We finished the presieve setup, return and start the complete bool sieve.
  return 0;
}

// Sets given segmented sieve
static void seg_sieve(uint32_t *primes, unsigned char *is_composite, uint64_t base_prime_count,
                      uint64_t low_num, uint64_t high_num, uint64_t *out_p_sum,
                      uint64_t *out_p_count) {

  // If it's the first block, we set the obvious ones to composite:
  if (low_num < 1) {
    is_composite[0] = 1;
    is_composite[1] = 1;
  }
  uint64_t segment_sum   = 0;
  uint64_t segment_count = 0;
  uint32_t segment_size  = high_num - low_num;

  // Then we iterate through every prime in primes[] that has a multiple within the segment.
  for (uint32_t i = 0; i < base_prime_count; i++) {
    uint64_t p         = primes[i];
    uint64_t p_squared = p * p;
    if (p_squared > high_num)
      break;

    // Start with first multiple of p that is >= low
    uint64_t start = low_num + ((p - (low_num % p)) % p);

    // If p^2 is greater than the multiple, just use that.
    if (start < p_squared) {
      start = p_squared;
    }

    // If we're beyond the high bound, just continue beyond.
    if (start > high_num) {
      continue;
    }

    // Finally calculate the offset from our multiple to the segment index.
    // and check off multiples until we hit the end of the sieve.
    for (uint32_t offset = start - low_num; offset < segment_size; offset += p) {
      is_composite[offset] = 1;
    }
  }

  // Accumulate sum and prime count from the composite sieve.
  for (uint32_t i = 0; i < segment_size; i++) {
    if (!is_composite[i]) {
      segment_sum += low_num + i;
      segment_count++;
    }
  }
  *out_p_count += segment_count;
  *out_p_sum += segment_sum;
}

// Square Root of end prime search w/ segmented bool array to find sum of all primes
// [2,  2^31] (or arg). Hopefully better than the old bool array version on ends
// greater than ~2^23.
int main(int argc, char *argv[]) {
  const char  *endpoint_arg         = NULL;
  _Bool        sum_only             = false;
  uint64_t     end                  = 1;
  uint32_t     start                = 3;
  uint64_t     prime_sum            = 0;
  uint64_t     total_primes_counter = 0;
  uint64_t     primes_capacity      = 64;      // Base primes array size.
  uint64_t     base_prime_count     = 0;       // Primes in sqrt(end) base sieve array.
  uint32_t     block_size           = 1 << 20; // Segment size (~1MB of unsigned char).
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
    // we convert from text to an uint64_t int with strtoull()
    if (parse_args(endpoint_arg, &end, &primes_capacity) != 0)
      return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Set endpoint to %lu\n", end);
    printf("Starting Search...\n");
  }

  // Late assignment so we don't have to free on each possible error before now.
  uint32_t *primes = malloc(primes_capacity * sizeof(uint32_t));

  // Handle errors
  if (primes == NULL) {
    printf("Error allocating primes array.\n");
    return EXIT_FAILURE;
  }

  // Now run the pre_sieve for the primes < sqrt(end)

  if (pre_sieve(&primes, &primes_capacity, &base_prime_count, start, end) != 0) {
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

  // Then set up a loop to run the sieves sequentially, and accumulate the intermediate sum & count.
  for (uint32_t i = 0;; i++) {

    // Set the high and low numbers.
    uint64_t low  = (uint64_t)i * block_size;
    uint64_t high = ((uint64_t)i + 1) * block_size;
    // Clear the bool array to zero (this also only touches the necessary bytes)
    memset(is_composite, 0, high - low);

    if (low > end) {
      // If the segment is jumps over the end, we're done.
      break;
    }
    if (high > end) {
      high = end + 1;
    }
    seg_sieve(primes, is_composite, base_prime_count, low, high, &prime_sum, &total_primes_counter);

    // Update handling
    current_update = high * updates / end;

    if (!sum_only && current_update > last_update && isatty(STDERR_FILENO)) {

      fprintf(stderr, "\rPrime count: %-10lu | Sum: %20lu | Completion %%: %5.2f",
              total_primes_counter, prime_sum, 100.0 * current_update / updates);
      fflush(stderr);
      last_update = current_update;
    }
  }

  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\rPrime count: %-10lu | Sum: %20lu | Completion %%: %3.2f \n",
            total_primes_counter, prime_sum, 100.00f);
    fflush(stderr);
  }

  if (!sum_only) {
    printf("Found %lu primes [0, %lu]\n", total_primes_counter, end);
    printf("Sum of found primes is %lu\n", prime_sum);
  } else {
    printf("%lu\n", prime_sum);
  }

  free(primes);
  free(is_composite);
  return 0;
}
