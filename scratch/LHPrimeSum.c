#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

__extension__ typedef unsigned __int128 u128;

typedef struct {
  uint64_t  n;
  uint64_t  root;
  size_t    count;
  uint64_t *values;
  u128     *sums;
  uint32_t *large_index;
} lh_sieve_t;

static uint64_t isqrt_u64(uint64_t n) {
  if (n < 2) {
    return n;
  }

  uint64_t x = n;
  uint64_t y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  return x;
}

static int parse_u64(const char *text, uint64_t *value_out) {
  errno                    = 0;
  char              *end   = NULL;
  unsigned long long value = strtoull(text, &end, 10);

  if (errno == ERANGE || end == text || *end != '\0') {
    return -1;
  }

  *value_out = (uint64_t)value;
  return 0;
}

static u128 sum_2_to(uint64_t n) {
  if (n < 2) {
    return 0;
  }

  return ((u128)n * (n + 1)) / 2 - 1;
}

static void print_u64_commas(uint64_t value) {
  char   digits[32];
  size_t length = 0;

  if (value == 0) {
    fputs("0", stdout);
    return;
  }

  while (value > 0) {
    digits[length++] = (char)('0' + (value % 10));
    value /= 10;
  }

  for (size_t left = 0; left < length; ++left) {
    if (left > 0 && ((length - left) % 3) == 0) {
      fputc(',', stdout);
    }
    fputc(digits[length - 1 - left], stdout);
  }
}

static void print_u128_commas(u128 value) {
  char   digits[64];
  size_t length = 0;

  if (value == 0) {
    fputs("0", stdout);
    return;
  }

  while (value > 0) {
    digits[length++] = (char)('0' + (value % 10));
    value /= 10;
  }

  for (size_t left = 0; left < length; ++left) {
    if (left > 0 && ((length - left) % 3) == 0) {
      fputc(',', stdout);
    }
    fputc(digits[length - 1 - left], stdout);
  }
}

static void print_u128_plain(u128 value) {
  char   digits[64];
  size_t length = 0;

  if (value == 0) {
    fputs("0", stdout);
    return;
  }

  while (value > 0) {
    digits[length++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while (length > 0) {
    fputc(digits[--length], stdout);
  }
}

static double monotonic_seconds(void) {
#ifdef _WIN32
  LARGE_INTEGER frequency;
  LARGE_INTEGER counter;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
#endif
}

static size_t count_quotient_values(uint64_t n, uint64_t root) {
  return (size_t)root + (size_t)(n / (root + 1));
}

static double estimate_gib(uint64_t root, size_t count) {
  const long double bytes = (long double)count * (long double)(sizeof(uint64_t) + sizeof(u128)) +
                            ((long double)root + 1.0L) * (long double)sizeof(uint32_t);
  return (double)(bytes / (1024.0L * 1024.0L * 1024.0L));
}

static size_t value_index(const lh_sieve_t *sieve, uint64_t value) {
  if (value <= sieve->root) {
    return sieve->count - (size_t)value;
  }

  return sieve->large_index[sieve->n / value];
}

static void free_lh_sieve(lh_sieve_t *sieve) {
  free(sieve->values);
  free(sieve->sums);
  free(sieve->large_index);
  memset(sieve, 0, sizeof *sieve);
}

static int init_lh_sieve(lh_sieve_t *sieve, uint64_t n, bool verbose) {
  memset(sieve, 0, sizeof *sieve);
  sieve->n     = n;
  sieve->root  = isqrt_u64(n);
  sieve->count = count_quotient_values(n, sieve->root);

  if (sieve->count > UINT32_MAX) {
    fputs("Too many quotient values for the compact index table.\n", stderr);
    return -1;
  }

  if (verbose) {
    printf("Quotient states: %zu\n", sieve->count);
    printf("Estimated table memory: %.2f GiB\n", estimate_gib(sieve->root, sieve->count));
  }

  sieve->values      = malloc(sieve->count * sizeof *sieve->values);
  sieve->sums        = malloc(sieve->count * sizeof *sieve->sums);
  sieve->large_index = malloc((sieve->root + 1) * sizeof *sieve->large_index);

  if (sieve->values == NULL || sieve->sums == NULL || sieve->large_index == NULL) {
    free_lh_sieve(sieve);
    fputs("Failed to allocate Lucy-Hedgehog tables.\n", stderr);
    return -1;
  }

  size_t index = 0;
  for (uint64_t low = 1; low <= n;) {
    const uint64_t value = n / low;
    const uint64_t high  = n / value;

    sieve->values[index] = value;
    sieve->sums[index]   = sum_2_to(value);

    if (value > sieve->root) {
      sieve->large_index[n / value] = (uint32_t)index;
    }

    ++index;
    if (high == UINT64_MAX) {
      break;
    }
    low = high + 1;
  }

  return 0;
}

static u128 prime_sum_lucy_hedgehog(lh_sieve_t *sieve) {
  if (sieve->n < 2) {
    return 0;
  }

  /*
   * sums[index(v)] starts as 2 + 3 + ... + v.
   *
   * After processing primes below p, sums[index(v)] has had composites with
   * smaller prime factors removed. If p is prime, subtract p times every
   * remaining value above p in the table for floor(v / p). This removes the
   * numbers whose smallest prime factor is p.
   */
  for (uint64_t p = 2; p <= sieve->root; ++p) {
    const size_t p_index        = value_index(sieve, p);
    const size_t before_p_index = value_index(sieve, p - 1);

    if (sieve->sums[p_index] == sieve->sums[before_p_index]) {
      continue;
    }

    const u128     sum_before_p = sieve->sums[before_p_index];
    const uint64_t p_squared    = p * p;

    for (size_t i = 0; i < sieve->count && sieve->values[i] >= p_squared; ++i) {
      const uint64_t reduced       = sieve->values[i] / p;
      const size_t   reduced_index = value_index(sieve, reduced);
      sieve->sums[i] -= (u128)p * (sieve->sums[reduced_index] - sum_before_p);
    }
  }

  return sieve->sums[0];
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Usage: %s <endpoint> [--estimate] [--sum-only]\n", argv[0]);
    return EXIT_FAILURE;
  }

  bool        estimate_only = false;
  bool        sum_only      = false;
  const char *endpoint_arg  = NULL;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--estimate") == 0) {
      estimate_only = true;
    } else if (strcmp(argv[i], "--sum-only") == 0) {
      sum_only = true;
    } else if (endpoint_arg == NULL) {
      endpoint_arg = argv[i];
    } else {
      fprintf(stderr, "Unknown argument: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  if (endpoint_arg == NULL || (estimate_only && sum_only)) {
    fprintf(stderr, "Usage: %s <endpoint> [--estimate] [--sum-only]\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint64_t n = 0;
  if (parse_u64(endpoint_arg, &n) != 0) {
    fprintf(stderr, "Invalid endpoint: %s\n", endpoint_arg);
    return EXIT_FAILURE;
  }

  if (!sum_only) {
    fputs("Computing exact sum of primes <= ", stdout);
    print_u64_commas(n);
    fputs(" with a Lucy-Hedgehog prime-sum sieve.\n", stdout);
  }

  const double start = monotonic_seconds();

  if (n < 2) {
    if (sum_only) {
      fputs("0\n", stdout);
    } else {
      fputs("Sum of primes <= ", stdout);
      print_u64_commas(n);
      fputs(" is 0\n", stdout);
      printf("Finished in %.6f seconds.\n", monotonic_seconds() - start);
    }
    return EXIT_SUCCESS;
  }

  if (estimate_only) {
    const uint64_t root  = isqrt_u64(n);
    const size_t   count = count_quotient_values(n, root);
    printf("Quotient states: %zu\n", count);
    printf("Estimated table memory: %.2f GiB\n", estimate_gib(root, count));
    return EXIT_SUCCESS;
  }

  lh_sieve_t sieve;
  if (init_lh_sieve(&sieve, n, !sum_only) != 0) {
    return EXIT_FAILURE;
  }

  const u128   answer = prime_sum_lucy_hedgehog(&sieve);
  const double finish = monotonic_seconds();

  if (sum_only) {
    print_u128_plain(answer);
    fputs("\n", stdout);
  } else {
    fputs("Sum of primes <= ", stdout);
    print_u64_commas(n);
    fputs(" is ", stdout);
    print_u128_commas(answer);
    fputs("\n", stdout);

    printf("Finished in %.6f seconds.\n", finish - start);
  }

  free_lh_sieve(&sieve);
  return EXIT_SUCCESS;
}
