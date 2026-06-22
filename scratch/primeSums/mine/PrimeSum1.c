#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Naive prime search and sum alg to find sum of all primes between 2 and 2^31 or arg.
// Store 'n sum is a bad process but just an excuse to learn how growing arrays works.
int main(int argc, char *argv[]) {
  const char        *endpoint_arg = NULL;
  _Bool              sum_only     = false;
  unsigned long      end          = 1;
  unsigned long      start        = 3;
  _Bool              composite;
  unsigned long      primes_counter = 0;
  unsigned long long prime_sum      = 0;
  // We'll start with an array of 64 and reallocate later by doubling as we need bigger arrays.
  // We have about 100M primes if we go to 2^31.
  unsigned long  primes_capacity = 64;
  unsigned long *primes          = malloc(primes_capacity * sizeof(*primes));
  if (primes == NULL) {
    printf("Error allocating primes array.");
    return EXIT_FAILURE;
  }

  if (argc != 2 && argc != 3) {
    // If we get too many arguments, exit with error
    printf("Usage: %s <endpoint> [--sum-only]\n", argv[0]);
    free(primes);
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--sum-only") == 0) {
      sum_only = true;
    } else if (endpoint_arg == NULL) {
      endpoint_arg = argv[i];
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      free(primes);
      return EXIT_FAILURE;
    }
  }

  if (endpoint_arg == NULL) {
    // If we didn't get an endpoint just use 2^31 by performing 31 bit shifts on 1.
    // Idiomatic I think.
    end = 1ull << 31;
    if (!sum_only)
      printf("No arguments found, setting endpoint to %lu\n", end);
  } else {
    // If we got an argument to sum to,
    // we convert from text to an unsigned long long int with strtoull()
    char *endptr;
    errno = 0;
    end   = strtoull(endpoint_arg, &endptr, 10);
    if (endptr == endpoint_arg || *endptr != '\0' || end < 2) {
      printf("Invalid endpoint: %s\n", endpoint_arg);
      free(primes);
      return EXIT_FAILURE;
    } else if (errno == ERANGE) {
      printf("Endpoint too large, use a smaller number. Exiting...");
      free(primes);
      return EXIT_FAILURE;
    }
    if (!sum_only)
      printf("Set endpoint to %lu\n", end);
  }
  if (!sum_only)
    printf("Starting Search...\n");

  // Set the obvious 2 as prime.
  primes[0] = 2ul;
  primes_counter++;

  for (unsigned long i = start; i <= end; i++) {
    // Skip evens
    if (i % 2 == 0) {
      continue;
    }
    // Reset flag.
    composite = false;
    // We setup a loop, and make sure to skip 2 since we already know i isn't even.
    // 3 should not run at all.
    for (unsigned long j = 1; j < primes_counter; j++) {
      // Check i against our primes list so far.
      if (i % primes[j] == 0) {
        // If divisible just break out immediately and set the 'composite' flag.
        composite = true;
        break;
      }
    }

    // If we made it out of the loop before without composite getting set, we found a prime.
    if (!composite) {
      // Now we check if primes is big enough to fit our number.
      if (primes_counter >= primes_capacity) {
        // If not, we expand to twice the size.
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
      // Set the latest prime
      primes[primes_counter] = i;
      // And update our counter for how many primes we've found.
      primes_counter++;
    }
  }

  // Finally we sum all the found primes.
  for (unsigned long i = 0; i < primes_counter; i++) {
    prime_sum += primes[i];
  }

  if (!sum_only) {
    printf("Found %lu primes from 2 to %lu inclusive.\n", primes_counter, end);
    printf("Sum of found primes is %llu\n", prime_sum);
  } else {
    printf("%llu\n", prime_sum);
  }

  free(primes);
  return 0;
}
