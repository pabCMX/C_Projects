#ifndef _WIN32
#define _DEFAULT_SOURCE
#include <stdint.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

static int parse_args(const char *text, uint64_t *end, uint32_t *primes_capacity) {
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
    *primes_capacity = isqrt(*end) / 2 + 16; // Lazy fudge for ok presize.
  return 0;
}

static int resize_array(uint32_t **array, uint32_t *capacity) {
  // Otherwise we expand by 2x primes_capacity.
  uint32_t new_capacity = *capacity * 2ul;

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

static int pre_sieve(uint32_t **primes, uint32_t *primes_capacity, uint32_t *base_prime_count,
                     uint32_t start, uint64_t end) {

  _Bool composite = false;
  // Set the obvious 2 as prime.
  (*primes)[0]      = 2u;
  *base_prime_count = 1u;

  for (uint64_t i = start;; i++) {
    // Skip evens
    if (i % 2 == 0) {
      continue;
    }

    uint64_t i_squared = i * i;
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

static uint32_t full_sieve(uint32_t *primes, uint32_t base_prime_count, uint64_t *prime_sum,
                           uint64_t end, _Bool sum_only) {

  uint32_t prime_count = 0;
  // First we create the flag sieve
  unsigned char *isComposite = calloc(end + 1, 1);
  if (isComposite == NULL) {
    printf("Error allocating flag sieve.\n");
    return 0;
  }
  // Then set the obvious ones to composite:
  isComposite[0] = 1;
  isComposite[1] = 1;
  // Then we iterate through every prime in primes[]
  if (!sum_only)
    printf("Starting composite marking...\n");
  for (uint32_t i = 0; i < base_prime_count; i++) {
    uint32_t p = primes[i];
    // Starting at p * p
    uint64_t p_mult = (uint64_t)p * p;

    // Until we hit the end of the sieve.
    while (p_mult <= end) {
      isComposite[p_mult] = 1;
      // Then walk to the next multiple.
      p_mult += p;
    }
    if (!sum_only && (i % 10000 == 0 || i < 1000) && isatty(STDERR_FILENO)) {
      fprintf(stderr, "\rCompletion %%: %3.2f", (i * 100.0) / base_prime_count);
      fflush(stderr);
    }
  }
  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\rCompletion %%: %3.2f\nStarting prime sum and count...\n", 100.0);
    fflush(stderr);
  }
  // Now we accumulate all primes:
  for (uint64_t i = 0; i <= end; i++) {
    if (!isComposite[i]) {
      *prime_sum += i;
      prime_count++;
    }
    if (!sum_only && i % 100000 == 0 && isatty(STDERR_FILENO)) {
      fprintf(stderr, "\rPrime count: %-10u | Sum: %20lu | Completion %%: %3.2f", prime_count,
              *prime_sum, (i * 100.0) / end);
      fflush(stderr);
    }
  }

  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\n");
    fflush(stderr);
  }
  free(isComposite);
  return prime_count;
}

// Square Root of end prime search for bool array multiple stride alg to find sum of all primes
// between 2 and 2^31 or given arg (inclusive). ~18.5x better than the old mod, count, and sum
// version on ends greater than ~2^23.
int main(int argc, char *argv[]) {
  const char *endpoint_arg         = NULL;
  _Bool       sum_only             = false;
  uint64_t    end                  = 1;
  uint32_t    start                = 3;
  uint64_t    prime_sum            = 0;
  uint64_t    total_primes_counter = 0;
  uint32_t    primes_capacity      = 64;
  uint32_t    base_prime_count     = 0;

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
    printf("Starting full sieve...\n");
  }

  // If we didn't have any issues, run the real sieve.
  total_primes_counter = full_sieve(primes, base_prime_count, &prime_sum, end, sum_only);
  if (total_primes_counter == 0) {
    free(primes);

    return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Found %lu primes from 2 to %lu inclusive.\n", total_primes_counter, end);
    printf("Sum of found primes is %lu\n", prime_sum);
  } else {
    printf("%lu\n", prime_sum);
  }

  free(primes);
  return 0;
}
