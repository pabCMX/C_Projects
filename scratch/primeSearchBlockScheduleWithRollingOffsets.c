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
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#endif

__extension__ typedef unsigned __int128 u128;

typedef struct {
    uint64_t end;
    uint64_t root;
    uint64_t segment_odd_count;
    uint64_t odd_index_limit;
    const uint32_t *base_primes;
    size_t base_prime_count;
    _Atomic int failed;
} sieve_work_t;

typedef struct {
    u128 sum;
    uint64_t count;
} worker_result_t;

typedef struct {
    sieve_work_t *work;
    worker_result_t *result;
    uint64_t range_start;
    uint64_t range_end;
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
    char *end = NULL;
    unsigned long long value = strtoull(text, &end, 10);

    if (end == text || *end != '\0') {
        return -1;
    }

    *endpoint = (uint64_t)value;
    return 0;
}

static void print_u64_commas(uint64_t value) {
    char digits[32];
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
    char digits[64];
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
    char digits[64];
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

static inline size_t bit_words_needed(size_t bit_count) {
    return (bit_count + 63u) / 64u;
}

/* 0 = prime candidate, 1 = composite. segment_odd_count is 2^21 (64-aligned). */
static inline void bit_words_clear_all(uint64_t *words, size_t nwords) {
    memset(words, 0, nwords * sizeof *words);
}

static inline void bit_mark_composite(uint64_t *words, size_t i) {
    words[i / 64] |= 1ull << (i % 64);
}

/* Inactive cursor: prime does not cross this thread's assigned range. */
static const uint64_t cross_inactive = UINT64_MAX;

static uint64_t first_cross_odd_index(uint64_t range_start_odd_index,
                                      uint64_t p) {
    const uint64_t low = 2 * range_start_odd_index + 1;
    const uint64_t p_squared = p * p;

    uint64_t start = ((low + p - 1) / p) * p;
    if (start < p_squared) {
        start = p_squared;
    }
    if ((start & 1) == 0) {
        start += p;
    }

    return (start - 1) / 2;
}

static void init_cross_cursors(uint64_t range_start, uint64_t range_end,
                               const uint32_t *base_primes,
                               size_t base_prime_count,
                               uint64_t *next_cross) {
    const uint64_t range_end_exclusive = 2 * range_end + 1;

    for (size_t i = 0; i < base_prime_count; ++i) {
        const uint64_t p = base_primes[i];
        if (p == 2) {
            next_cross[i] = cross_inactive;
            continue;
        }

        if (p * p >= range_end_exclusive) {
            for (size_t j = i; j < base_prime_count; ++j) {
                next_cross[j] = cross_inactive;
            }
            return;
        }

        const uint64_t first = first_cross_odd_index(range_start, p);
        next_cross[i] =
            first >= range_end ? cross_inactive : first;
    }
}

static void sum_segment_primes(const uint64_t *words, size_t size, uint64_t low,
                               u128 *sum_out, uint64_t *count_out) {
    const size_t nwords = bit_words_needed(size);
    const size_t extra = nwords * 64u - size;
    const uint64_t tail_mask = extra == 0 ? ~0ull : (~0ull >> extra);

    u128 sum = 0;
    uint64_t prime_count = 0;

    for (size_t w = 0; w < nwords; ++w) {
        uint64_t prime_bits = ~words[w];
        if (w + 1 == nwords) {
            prime_bits &= tail_mask;
        }

        while (prime_bits) {
            const unsigned bit = (unsigned)__builtin_ctzll(prime_bits);
            prime_bits &= prime_bits - 1;
            const size_t offset = w * 64 + bit;
            sum += (u128)(low + 2 * (uint64_t)offset);
            ++prime_count;
        }
    }

    *sum_out = sum;
    *count_out = prime_count;
}

static void sieve_odd_segment(uint64_t low_index, uint64_t high_index,
                              const uint32_t *base_primes,
                              size_t base_prime_count, uint64_t *words,
                              uint64_t *next_cross, u128 *sum_out,
                              uint64_t *count_out) {
    const size_t size = (size_t)(high_index - low_index);
    if (size == 0) {
        *sum_out = 0;
        *count_out = 0;
        return;
    }

    const size_t nwords = bit_words_needed(size);
    bit_words_clear_all(words, nwords);

    const uint64_t low = 2 * low_index + 1;
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

        uint64_t cross = next_cross[i];
        if (cross == cross_inactive) {
            continue;
        }

        for (; cross < high_index; cross += p) {
            bit_mark_composite(words, (size_t)(cross - low_index));
        }
        next_cross[i] = cross;
    }

    sum_segment_primes(words, size, low, sum_out, count_out);
}

static size_t sieve_primes_upto(uint64_t limit, uint32_t **primes_out) {
    if (limit < 2) {
        *primes_out = NULL;
        return 0;
    }

    const size_t sieve_size = (size_t)limit + 1;
    bool *is_prime = malloc(sieve_size * sizeof *is_prime);
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

#ifdef _WIN32
static DWORD WINAPI sieve_worker(LPVOID arg) {
#else
static void *sieve_worker(void *arg) {
#endif
    worker_arg_t *worker = arg;
    sieve_work_t *work = worker->work;
    u128 local_sum = 0;
    uint64_t local_count = 0;

    const size_t segment_words =
        bit_words_needed((size_t)work->segment_odd_count);
    uint64_t *words = malloc(segment_words * sizeof *words);
    uint64_t *next_cross =
        malloc(work->base_prime_count * sizeof *next_cross);
    if (words == NULL || next_cross == NULL) {
        free(next_cross);
        free(words);
        atomic_store(&work->failed, 1);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }

    init_cross_cursors(worker->range_start, worker->range_end, work->base_primes,
                       work->base_prime_count, next_cross);

    for (uint64_t low_index = worker->range_start; low_index < worker->range_end;) {
        if (atomic_load(&work->failed) != 0) {
            break;
        }

        uint64_t high_index = low_index + work->segment_odd_count;
        if (high_index > worker->range_end) {
            high_index = worker->range_end;
        }

        u128 segment_sum = 0;
        uint64_t segment_count = 0;
        sieve_odd_segment(low_index, high_index, work->base_primes,
                          work->base_prime_count, words, next_cross,
                          &segment_sum, &segment_count);

        local_sum += segment_sum;
        local_count += segment_count;
        low_index = high_index;
    }

    free(next_cross);
    free(words);
    worker->result->sum = local_sum;
    worker->result->count = local_count;

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static int read_sysfs_int(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }

    int value = -1;
    if (fscanf(file, "%d", &value) != 1) {
        value = -1;
    }

    fclose(file);
    return value;
}

static bool sysfs_cpu_topology_path(char *path, size_t path_size,
                                    const char *cpu_name,
                                    const char *field) {
    static const char prefix[] = "/sys/devices/system/cpu/";
    static const char middle[] = "/topology/";
    const size_t field_len = strlen(field);
    const size_t fixed_len =
        (sizeof prefix - 1) + (sizeof middle - 1) + field_len;

    if (path_size <= fixed_len) {
        return false;
    }

    const int name_max = (int)(path_size - fixed_len - 1);
    const int written = snprintf(
        path, path_size, "%s%.*s%s%s", prefix, name_max, cpu_name, middle,
        field);

    return written > 0 && (size_t)written < path_size;
}

static int physical_core_count(void) {
#ifdef _WIN32
    DWORD buffer_length = 0;
    GetLogicalProcessorInformation(NULL, &buffer_length);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return 1;
    }

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer = malloc(buffer_length);
    if (buffer == NULL) {
        return 1;
    }

    if (!GetLogicalProcessorInformation(buffer, &buffer_length)) {
        free(buffer);
        return 1;
    }

    int cores = 0;
    const DWORD count =
        buffer_length / (DWORD)sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (DWORD i = 0; i < count; ++i) {
        if (buffer[i].Relationship == RelationProcessorCore) {
            ++cores;
        }
    }

    free(buffer);
    return cores > 0 ? cores : 1;
#else
    DIR *dir = opendir("/sys/devices/system/cpu");
    if (dir != NULL) {
        struct core_key {
            int package_id;
            int core_id;
        } cores[1024];
        int core_count = 0;

        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != 'c' ||
                strncmp(entry->d_name, "cpu", 3) != 0) {
                continue;
            }

            const char *suffix = entry->d_name + 3;
            if (*suffix == '\0') {
                continue;
            }

            for (const char *ch = suffix; *ch != '\0'; ++ch) {
                if (!isdigit((unsigned char)*ch)) {
                    goto next_cpu;
                }
            }

            {
                char path[256];
                if (!sysfs_cpu_topology_path(path, sizeof path, entry->d_name,
                                             "physical_package_id")) {
                    goto next_cpu;
                }
                const int package_id = read_sysfs_int(path);
                if (!sysfs_cpu_topology_path(path, sizeof path, entry->d_name,
                                             "core_id")) {
                    goto next_cpu;
                }
                const int core_id = read_sysfs_int(path);
                if (package_id < 0 || core_id < 0) {
                    goto next_cpu;
                }

                int found = 0;
                for (int i = 0; i < core_count; ++i) {
                    if (cores[i].package_id == package_id &&
                        cores[i].core_id == core_id) {
                        found = 1;
                        break;
                    }
                }

                if (!found && core_count < (int)(sizeof cores / sizeof cores[0])) {
                    cores[core_count].package_id = package_id;
                    cores[core_count].core_id = core_id;
                    ++core_count;
                }
            }

        next_cpu:
            continue;
        }

        closedir(dir);
        if (core_count > 0) {
            return core_count;
        }
    }

    {
        const long logical = sysconf(_SC_NPROCESSORS_ONLN);
        return logical > 0 ? (int)logical : 1;
    }
#endif
}

static int worker_thread_count(void) {
    int cores = physical_core_count();

#ifdef _WIN32
    if (cores > MAXIMUM_WAIT_OBJECTS) {
        cores = MAXIMUM_WAIT_OBJECTS;
    }
#endif

    if (cores <= 1) {
        return 1;
    }

    return cores - 1;
}

static uint64_t min_u64(uint64_t a, uint64_t b) {
    return a < b ? a : b;
}

static int run_prime_search(uint64_t end, int threads, bool verbose,
                            u128 *sum_out, uint64_t *count_out) {
    if (end < 2) {
        *sum_out = 0;
        *count_out = 0;
        return 0;
    }

    const uint64_t root = isqrt_u64(end);
    /* 2^21 odd slots -> 256 KiB bit buffer; 64-bit aligned, no init tail mask. */
    const uint64_t segment_odd_count = 1ULL << 21;
    const uint64_t odd_index_limit = (end + 1) / 2;

    if (verbose) {
        fputs("Finding intial prime factors.", stdout);
        fflush(stdout);
    }

    uint32_t *base_primes = NULL;
    const size_t base_prime_count = sieve_primes_upto(root, &base_primes);
    if (base_prime_count == SIZE_MAX) {
        fputs("\nFailed to allocate base sieve.\n", stderr);
        return -1;
    }

    if (verbose) {
        fputs("\tDone.\n", stdout);
    }

    u128 total_sum = 0;
    uint64_t total_count = 0;
    if (end >= 2) {
        total_sum = 2;
        total_count = 1;
    }

    if (verbose) {
        fputs("Implementing full Sieve - Searching...", stdout);
        fflush(stdout);
    }

    sieve_work_t work = {
        .end = end,
        .root = root,
        .segment_odd_count = segment_odd_count,
        .odd_index_limit = odd_index_limit,
        .base_primes = base_primes,
        .base_prime_count = base_prime_count,
        .failed = 0,
    };

    worker_result_t *results = calloc((size_t)threads, sizeof *results);
    worker_arg_t *args = calloc((size_t)threads, sizeof *args);
    if (results == NULL || args == NULL) {
        free(args);
        free(results);
        free(base_primes);
        fputs("\nFailed to allocate worker state.\n", stderr);
        return -1;
    }

    const uint64_t searchable = odd_index_limit - 1;
    const uint64_t chunk =
        (searchable + (uint64_t)threads - 1) / (uint64_t)threads;

    for (int i = 0; i < threads; ++i) {
        const uint64_t range_start = 1 + (uint64_t)i * chunk;
        uint64_t range_end = 1 + (uint64_t)(i + 1) * chunk;
        if (range_end > odd_index_limit) {
            range_end = odd_index_limit;
        }

        args[i].work = &work;
        args[i].result = &results[i];
        args[i].range_start = min_u64(range_start, odd_index_limit);
        args[i].range_end = min_u64(range_end, odd_index_limit);
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
        handles[i] = CreateThread(NULL, 0, sieve_worker, &args[i], 0, NULL);
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
    *sum_out = total_sum;
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

    if (!GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time,
                         &kernel_time, &user_time)) {
        return 0.0;
    }

    const ULONGLONG kernel_ticks =
        ((ULONGLONG)kernel_time.dwHighDateTime << 32) |
        (ULONGLONG)kernel_time.dwLowDateTime;
    const ULONGLONG user_ticks =
        ((ULONGLONG)user_time.dwHighDateTime << 32) |
        (ULONGLONG)user_time.dwLowDateTime;

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
    const unsigned long minutes = (unsigned long)(seconds / 60.0);
    const double remainder = seconds - (60.0 * (double)minutes);
    printf("%s %lu minutes and %.2f seconds.\n", label, minutes, remainder);
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s <endpoint> [--sum-only]\n", argv[0]);
        return EXIT_FAILURE;
    }

    bool sum_only = false;
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

    const double cpu_start = process_cpu_seconds();
    const double wall_start = monotonic_seconds();

    u128 prime_sum = 0;
    uint64_t prime_count = 0;
    if (run_prime_search(end, threads, !sum_only, &prime_sum, &prime_count) !=
        0) {
        return EXIT_FAILURE;
    }

    const double wall_elapsed = monotonic_seconds() - wall_start;
    const double cpu_elapsed = process_cpu_seconds() - cpu_start;

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
