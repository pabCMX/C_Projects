#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

__extension__ typedef unsigned __int128 u128;

typedef struct {
  uint64_t         end;
  uint64_t         root;
  uint64_t         segment_odd_count;
  uint64_t         odd_index_limit;
  const uint32_t  *base_primes;
  size_t           base_prime_count;
  _Atomic uint64_t next_odd_index;
  _Atomic int      failed;
} sieve_work_t;

typedef struct {
  u128     sum;
  uint64_t count;
} worker_result_t;

typedef struct {
  sieve_work_t    *work;
  worker_result_t *result;
} worker_arg_t;

static uint64_t isqrt_u64(uint64_t n) {
  if (n < 2) {
    return 0;
  }

  uint64_t x = n;
  uint64_t y = (x + 1) / 2;

  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }

  return x;
}

static int parse_endpoint(const char *text, uint64_t *endpoint) {
  char              *end   = NULL;
  unsigned long long value = strtoull(text, &end, 10);

  if (end == text || *end != '\0') {
    return -1;
  }

  *endpoint = (uint64_t)value;
  return 0;
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

static size_t sieve_primes_upto(uint64_t limit, uint32_t **primes_out) {
  if (limit < 2) {
    *primes_out = NULL;
    return 0;
  }

  const size_t sieve_size = (size_t)limit + 1;
  bool        *is_prime   = malloc(sieve_size * sizeof *is_prime);
  if (is_prime == NULL) {
    return SIZE_MAX;
  }

  memset(is_prime, 1, sieve_size);
  is_prime[0] = false;
  is_prime[1] = false;

  const uint64_t stop = isqrt_u64(limit);
  for (uint64_t p = 2; p <= stop; ++p) {
    if (!is_prime[p]) {
      continue;
    }

    const uint64_t start = p * p;
    for (uint64_t multiple = start; multiple <= limit; multiple += p) {
      is_prime[multiple] = false;
    }
  }

  size_t count = 0;
  for (uint64_t n = 2; n <= limit; ++n) {
    if (is_prime[n]) {
      ++count;
    }
  }

  uint32_t *primes = malloc(count * sizeof *primes);
  if (primes == NULL) {
    free(is_prime);
    return SIZE_MAX;
  }

  size_t index = 0;
  for (uint64_t n = 2; n <= limit; ++n) {
    if (is_prime[n]) {
      primes[index++] = (uint32_t)n;
    }
  }

  free(is_prime);
  *primes_out = primes;
  return count;
}

static void sieve_odd_segment(uint64_t low_index, uint64_t high_index, const uint32_t *base_primes,
                              size_t base_prime_count, unsigned char *is_prime, u128 *sum_out,
                              uint64_t *count_out) {
  const size_t size = (size_t)(high_index - low_index);
  if (size == 0) {
    *sum_out   = 0;
    *count_out = 0;
    return;
  }

  memset(is_prime, 1, size);

  const uint64_t low            = 2 * low_index + 1;
  const uint64_t high_exclusive = 2 * high_index + 1;

  for (size_t i = 0; i < base_prime_count; ++i) {
    const uint64_t p = base_primes[i];
    if (p == 2) {
      continue;
    }

    const uint64_t p_squared = p * p;

    if (p_squared >= high_exclusive) {
      break;
    }

    uint64_t start = ((low + p - 1) / p) * p;
    if (start < p_squared) {
      start = p_squared;
    }
    if ((start & 1) == 0) {
      start += p;
    }
    if (start >= high_exclusive) {
      continue;
    }

    size_t offset = (size_t)((start - low) / 2);
    for (; offset < size; offset += (size_t)p) {
      is_prime[offset] = 0;
    }
  }

  u128     sum         = 0;
  uint64_t prime_count = 0;

  for (size_t offset = 0; offset < size; ++offset) {
    if (!is_prime[offset]) {
      continue;
    }

    const uint64_t prime = low + 2 * (uint64_t)offset;
    sum += (u128)prime;
    ++prime_count;
  }

  *sum_out   = sum;
  *count_out = prime_count;
}

#ifdef _WIN32
static DWORD WINAPI sieve_worker(LPVOID arg) {
#else
static void *sieve_worker(void *arg) {
#endif
  worker_arg_t *worker      = arg;
  sieve_work_t *work        = worker->work;
  u128          local_sum   = 0;
  uint64_t      local_count = 0;

  unsigned char *is_prime = malloc((size_t)work->segment_odd_count * sizeof *is_prime);
  if (is_prime == NULL) {
    atomic_store(&work->failed, 1);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
  }

  for (;;) {
    if (atomic_load(&work->failed) != 0) {
      break;
    }

    const uint64_t low_index = atomic_fetch_add(&work->next_odd_index, work->segment_odd_count);
    if (low_index >= work->odd_index_limit) {
      break;
    }

    uint64_t high_index = low_index + work->segment_odd_count;
    if (high_index > work->odd_index_limit) {
      high_index = work->odd_index_limit;
    }

    u128     segment_sum   = 0;
    uint64_t segment_count = 0;
    sieve_odd_segment(low_index, high_index, work->base_primes, work->base_prime_count, is_prime,
                      &segment_sum, &segment_count);

    local_sum += segment_sum;
    local_count += segment_count;
  }

  free(is_prime);
  worker->result->sum   = local_sum;
  worker->result->count = local_count;

#ifdef _WIN32
  return 0;
#else
  return NULL;
#endif
}

static int worker_thread_count(void) {
  unsigned int cores = 1;

#ifdef _WIN32
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  cores = info.dwNumberOfProcessors;
  if (cores > MAXIMUM_WAIT_OBJECTS) {
    cores = MAXIMUM_WAIT_OBJECTS;
  }
#else
  long detected = sysconf(_SC_NPROCESSORS_ONLN);
  if (detected > 0) {
    cores = (unsigned int)detected;
  }
#endif

  if (cores <= 1) {
    return 1;
  }

  return (int)(cores - 1);
}

static int run_prime_search(uint64_t end, int threads, bool verbose, u128 *sum_out,
                            uint64_t *count_out) {
  if (end < 2) {
    *sum_out   = 0;
    *count_out = 0;
    return 0;
  }

  const uint64_t root              = isqrt_u64(end);
  const uint64_t segment_odd_count = 1ULL << 22;
  const uint64_t odd_index_limit   = (end + 1) / 2;

  if (verbose) {
    fputs("Finding intial prime factors.", stdout);
    fflush(stdout);
  }

  uint32_t    *base_primes      = NULL;
  const size_t base_prime_count = sieve_primes_upto(root, &base_primes);
  if (base_prime_count == SIZE_MAX) {
    fputs("\nFailed to allocate base sieve.\n", stderr);
    return -1;
  }

  if (verbose) {
    fputs("\tDone.\n", stdout);
  }

  u128     total_sum   = 0;
  uint64_t total_count = 0;
  if (end >= 2) {
    total_sum   = 2;
    total_count = 1;
  }

  if (verbose) {
    fputs("Implementing full Sieve - Searching...", stdout);
    fflush(stdout);
  }

  sieve_work_t work = {
      .end               = end,
      .root              = root,
      .segment_odd_count = segment_odd_count,
      .odd_index_limit   = odd_index_limit,
      .base_primes       = base_primes,
      .base_prime_count  = base_prime_count,
      .next_odd_index    = 1,
      .failed            = 0,
  };

  worker_result_t *results = calloc((size_t)threads, sizeof *results);
  worker_arg_t    *args    = calloc((size_t)threads, sizeof *args);
  if (results == NULL || args == NULL) {
    free(args);
    free(results);
    free(base_primes);
    fputs("\nFailed to allocate worker state.\n", stderr);
    return -1;
  }

#ifdef _WIN32
  HANDLE *handles = calloc((size_t)threads, sizeof *handles);
  if (handles == NULL) {
    free(args);
    free(results);
    free(base_primes);
    fputs("\nFailed to allocate worker handles.\n", stderr);
    return -1;
  }

  for (int i = 0; i < threads; ++i) {
    args[i].work   = &work;
    args[i].result = &results[i];
    handles[i]     = CreateThread(NULL, 0, sieve_worker, &args[i], 0, NULL);
    if (handles[i] == NULL) {
      for (int j = 0; j < i; ++j) {
        WaitForSingleObject(handles[j], INFINITE);
        CloseHandle(handles[j]);
      }
      free(handles);
      free(args);
      free(results);
      free(base_primes);
      fputs("\nFailed to create worker thread.\n", stderr);
      return -1;
    }
  }

  WaitForMultipleObjects((DWORD)threads, handles, TRUE, INFINITE);
  for (int i = 0; i < threads; ++i) {
    CloseHandle(handles[i]);
  }
  free(handles);
#else
  pthread_t *workers = calloc((size_t)threads, sizeof *workers);
  if (workers == NULL) {
    free(args);
    free(results);
    free(base_primes);
    fputs("\nFailed to allocate worker threads.\n", stderr);
    return -1;
  }

  for (int i = 0; i < threads; ++i) {
    args[i].work   = &work;
    args[i].result = &results[i];
    if (pthread_create(&workers[i], NULL, sieve_worker, &args[i]) != 0) {
      for (int j = 0; j < i; ++j) {
        pthread_join(workers[j], NULL);
      }
      free(workers);
      free(args);
      free(results);
      free(base_primes);
      fputs("\nFailed to create worker thread.\n", stderr);
      return -1;
    }
  }

  for (int i = 0; i < threads; ++i) {
    pthread_join(workers[i], NULL);
  }
  free(workers);
#endif

  if (verbose) {
    fputs("\tDone.\n", stdout);
    fputs("Saving results.\tDone.\n", stdout);
  }

  if (atomic_load(&work.failed) != 0) {
    free(args);
    free(results);
    free(base_primes);
    fputs("\nFailed to allocate worker sieve buffer.\n", stderr);
    return -1;
  }

  for (int i = 0; i < threads; ++i) {
    total_sum += results[i].sum;
    total_count += results[i].count;
  }

  free(args);
  free(results);
  free(base_primes);
  *sum_out   = total_sum;
  *count_out = total_count;
  return 0;
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

static double process_cpu_seconds(void) {
#ifdef _WIN32
  FILETIME create_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;

  if (!GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time)) {
    return 0.0;
  }

  const ULONGLONG kernel_ticks =
      ((ULONGLONG)kernel_time.dwHighDateTime << 32) | (ULONGLONG)kernel_time.dwLowDateTime;
  const ULONGLONG user_ticks =
      ((ULONGLONG)user_time.dwHighDateTime << 32) | (ULONGLONG)user_time.dwLowDateTime;

  return (double)(kernel_ticks + user_ticks) / 10000000.0;
#else
#if defined(CLOCK_PROCESS_CPUTIME_ID)
  struct timespec ts;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
  }
#endif
  return (double)clock() / (double)CLOCKS_PER_SEC;
#endif
}

static void print_elapsed(const char *label, double seconds) {
  const unsigned long minutes   = (unsigned long)(seconds / 60.0);
  const double        remainder = seconds - (60.0 * (double)minutes);
  printf("%s %lu minutes and %.2f seconds.\n", label, minutes, remainder);
}

int main(int argc, char *argv[]) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "Usage: %s <endpoint> [--sum-only]\n", argv[0]);
    return EXIT_FAILURE;
  }

  bool        sum_only     = false;
  const char *endpoint_arg = NULL;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--sum-only") == 0) {
      sum_only = true;
    } else if (endpoint_arg == NULL) {
      endpoint_arg = argv[i];
    } else {
      fprintf(stderr, "Unknown argument: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  if (endpoint_arg == NULL) {
    fprintf(stderr, "Usage: %s <endpoint> [--sum-only]\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint64_t end = 0;
  if (parse_endpoint(endpoint_arg, &end) != 0) {
    fprintf(stderr, "Invalid endpoint: %s\n", endpoint_arg);
    return EXIT_FAILURE;
  }

  const int threads = worker_thread_count();

  if (!sum_only) {
    fputs("Finding all primes between 0 and ", stdout);
    print_u64_commas(end);
    fputs("\n", stdout);
  }

  const double cpu_start  = process_cpu_seconds();
  const double wall_start = monotonic_seconds();

  u128     prime_sum   = 0;
  uint64_t prime_count = 0;
  if (run_prime_search(end, threads, !sum_only, &prime_sum, &prime_count) != 0) {
    return EXIT_FAILURE;
  }

  const double wall_elapsed = monotonic_seconds() - wall_start;
  const double cpu_elapsed  = process_cpu_seconds() - cpu_start;

  if (sum_only) {
    print_u128_plain(prime_sum);
    fputs("\n", stdout);
    return EXIT_SUCCESS;
  }

  if (prime_count == 0) {
    fputs("No primes found, exiting\n", stdout);
    return EXIT_SUCCESS;
  }

  print_elapsed("Finished prime search in", wall_elapsed);
  print_elapsed("CPU time:", cpu_elapsed);

  fputs("Now totaling all primes between 0 and ", stdout);
  print_u64_commas(end);
  fputs("\n", stdout);

  fputs("Sum of ", stdout);
  print_u64_commas(prime_count);
  fputs(" primes from 0 to ", stdout);
  print_u64_commas(end);
  fputs(" is ", stdout);
  print_u128_commas(prime_sum);
  fputs("\n", stdout);

  return EXIT_SUCCESS;
}
