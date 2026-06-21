#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
// Naive prime search and sum alg to find sum of all primes between 2 and 2^31 or arg.
// Store 'n sum is a bad process but just an excuse to learn how growing arrays works.
int main(int argc, char *argv[]) {
  const char        *endpoint_arg = NULL;
  _Bool              atty_good    = false;
  _Bool              sum_only     = false;
  unsigned long      end          = 1;
  unsigned long      start        = 3;
  _Bool              composite;
  unsigned long      primes_array_counter = 0;
  unsigned long long prime_sum            = 0;
  unsigned long      total_primes_counter = 0;
  unsigned long      primes_capacity      = 64;

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

  if (isatty(STDERR_FILENO)) {
    atty_good = true;
  }

  if (endpoint_arg == NULL) {
    // If we didn't get an endpoint just use 2^31 by performing 31 bit shifts on 1.
    // Idiomatic I think.
    end = 1ull << 31;
    if (!sum_only)
      printf("No arguments found\nSet endpoint to %lu\n", end);
  } else {
    // If we got an argument to sum to,
    // we convert from text to an unsigned long long int with strtoull()
    char *endptr;
    errno           = 0;
    end             = strtoull(endpoint_arg, &endptr, 10);
    primes_capacity = isqrt(end) / 2 + 16; // Lazy fudge for ok presize.
    if (endptr == endpoint_arg || *endptr != '\0' || end < 2) {
      printf("Invalid endpoint: %s\n", endpoint_arg);
      return EXIT_FAILURE;
    } else if (errno == ERANGE) {
      printf("Endpoint too large, use a smaller number. Exiting...");
      return EXIT_FAILURE;
    }
    if (!sum_only)
      printf("Set endpoint to %lu\n", end);
  }
  if (!sum_only)
    printf("Starting Search...\n");

  // Late assignment so we don't have to free on each possible error before now.
  unsigned long *primes = malloc(primes_capacity * sizeof(*primes));

  if (primes == NULL) {
    printf("Error allocating primes array.\n");
    return EXIT_FAILURE;
  }

  // Set the obvious 2 as prime.
  primes[0] = 2ul;
  prime_sum += 2ul;
  primes_array_counter++;
  total_primes_counter++;

  for (unsigned long i = start; i <= end; i++) {
    // Skip evens
    if (i % 2 == 0) {
      continue;
    }
    // Reset flag.
    composite = false;
    // We setup a loop, and make sure to skip 2 since we already know i isn't even.
    // 3 should not run at all.
    for (unsigned long j = 1; j < primes_array_counter; j++) {
      // Check i against our primes list so far.
      if (i % primes[j] == 0) {
        // If divisible just break out immediately and set the 'composite' flag.
        composite = true;
        break;
      } else if (primes[j] * primes[j] > i) {
        // If the prime is too big, it won't be a factor anyways, and we can end early.
        break;
      }
    }

    // If we made it out of the loop before without composite getting set, we found a prime.
    if (!composite) {
      // Now we check if *primes[] is big enough to fit our new number. Just in case.
      if (primes_array_counter >= primes_capacity) {
        // Otherwise we expand by 2x primes_capacity.
        unsigned long new_capacity = primes_capacity * 2;

        // Create a temporary array *pointer* to a bigger array.
        unsigned long *tmp = realloc(primes, new_capacity * sizeof(*primes));

        // Handle errors.
        if (tmp == NULL) {
          free(primes);
          printf("Unable to reallocate primes array. Exiting...\n");
          return EXIT_FAILURE;
        }

        // Finally set primes pointer to new array, and primes_capacity to new_capacity.
        primes          = tmp;
        primes_capacity = new_capacity;
      }

      if (i * i <= end) {
        // Set the latest prime
        primes[primes_array_counter] = i;
        // And update our counter for how many primes we've found.
        primes_array_counter++;
      }

      // Finally we accumulate the found prime.
      prime_sum += i;
      total_primes_counter++;
      if (!sum_only && total_primes_counter % 100000 == 0 && atty_good) {
        fprintf(stderr, "\rPrime count: %-10lu | Sum: %20llu | Completion %%: %3.2f",
                total_primes_counter, prime_sum, (i * 100.0) / end);
        fflush(stderr);
      }
    }
  }

  if (!sum_only) {
    if (atty_good)
      fprintf(stderr, "\n");
    printf("Found %lu primes from 2 to %lu inclusive.\n", total_primes_counter, end);
    printf("Sum of found primes is %llu\n", prime_sum);
  } else {
    printf("%llu\n", prime_sum);
  }

  free(primes);
  return 0;
}
