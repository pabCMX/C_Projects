#ifndef _WIN32
#define _DEFAULT_SOURCE
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long isqrt(unsigned long n) {
  if (n == 0)
    return 0;

  unsigned long lo = 1;
  unsigned long hi = n;

  while (lo < hi) {
    unsigned long mid = lo + (hi - lo + 1) / 2;
    if (mid <= n / mid) {
      lo = mid;
    } else {
      hi = mid - 1;
    }
  }
  return lo;
}

static int parse_args(const char *text, unsigned long *end, unsigned long *primes_capacity) {
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

static int resize_array(unsigned long **array, unsigned long *capacity) {
  // Otherwise we expand by 2x primes_capacity.
  unsigned long new_capacity = *capacity * 2ul;

  // Create a temporary array *pointer* to a bigger array.
  unsigned long *tmp = realloc(*array, new_capacity * sizeof(**array));

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
static int pre_sieve(unsigned long **primes, unsigned long *primes_capacity,
                     unsigned long *base_prime_count, unsigned long start, unsigned long end) {

  _Bool composite = false;
  // Set the obvious 2 as prime.
  *primes[0]        = 2ul;
  *base_prime_count = 1ul;

  for (unsigned long i = start; i * i <= end; i++) {
    // Skip evens
    if (i % 2 == 0) {
      continue;
    }
    // Reset flag.
    composite = false;
    // We setup a loop, and make sure to skip 2 since we already know i isn't even.
    // 3 should not run at all.
    for (unsigned long j = 1; j < *base_prime_count; j++) {
      // Check i against our primes list so far.
      if (i % *primes[j] == 0) {
        // If divisible just break out immediately and set the 'composite' flag.
        composite = true;
        break;
      } else if (*primes[j] * *primes[j] > i) {
        // If the prime is too big, it won't be a factor anyways, and we can end early.
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
      *primes[*base_prime_count] = i;
      // And update our counters for how many primes we've found.
      (*base_prime_count)++;
    }
  }
  // We finished the presieve setup, return and start the complete bool sieve.
  return 0;
}

static unsigned long full_sieve(unsigned long *primes, unsigned long base_prime_count,
                                unsigned long long *prime_sum, unsigned long end, _Bool sum_only) {

  unsigned long prime_count = 0;
  size_t        size        = ((end + 1) * sizeof(int));
  // First we create the flag sieve
  int *isComposite = calloc(size, 1);
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
  for (unsigned long i = 0; i < base_prime_count; i++) {
    unsigned long p = primes[i];
    // Starting at p * p
    unsigned long long p_mult = (unsigned long long)p * p;

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
  for (unsigned long i = 0; i <= end; i++) {
    if (!isComposite[i]) {
      *prime_sum += i;
      prime_count++;
    }
    if (!sum_only && i % 100000 == 0 && isatty(STDERR_FILENO)) {
      fprintf(stderr, "\rPrime count: %-10lu | Sum: %20llu | Completion %%: %3.2f", prime_count,
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
  const char        *endpoint_arg         = NULL;
  _Bool              sum_only             = false;
  unsigned long      end                  = 1;
  unsigned long      start                = 3;
  unsigned long long prime_sum            = 0;
  unsigned long      total_primes_counter = 0;
  unsigned long      primes_capacity      = 64;
  unsigned long      base_prime_count     = 0;

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
    // we convert from text to an unsigned long long int with strtoull()
    if (parse_args(endpoint_arg, &end, &primes_capacity) != 0)
      return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Set endpoint to %lu\n", end);
    printf("Starting Search...\n");
  }

  // Late assignment so we don't have to free on each possible error before now.
  unsigned long *primes = malloc(primes_capacity * sizeof(*primes));

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
    printf("Sum of found primes is %llu\n", prime_sum);
  } else {
    printf("%llu\n", prime_sum);
  }

  free(primes);
  return 0;
}
