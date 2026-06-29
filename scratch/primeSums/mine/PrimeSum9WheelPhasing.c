#ifndef _WIN32
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

__extension__ typedef unsigned __int128 u128;

typedef struct {
  uint32_t prime;
} WheelPrimes;

typedef struct {
  uint32_t prime;
  uint64_t cursor;
} PrimeState;

static const WheelPrimes W_PRIMES[] = {
    {.prime = 3},  {.prime = 5},  {.prime = 7},  {.prime = 11}, {.prime = 13}, {.prime = 17},
    {.prime = 19}, {.prime = 23}, {.prime = 31}, {.prime = 37}, {.prime = 41}, {.prime = 43},
    {.prime = 47}, {.prime = 53}, {.prime = 59}, {.prime = 61},
};

static const uint32_t W_PRIME_COUNT =
    5; // Count of primes used from the wheel struct to make the pre-sieve pattern.

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
    // cast it to a char. Since characters are treated as ints, we can 'add' to get the right
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

// Uses a binary search to find the last active prime index for the segment, then returns the count.
static uint32_t count_active_primes_binary(uint32_t *primes, uint64_t base_prime_count,
                                           uint64_t high_num) {
  if (base_prime_count < 1)
    return 0; // We don't have any base primes to iterate, so return 0
  uint32_t low  = 0;
  uint32_t high = base_prime_count - 1;
  uint32_t mid  = 0;
  while (low < high) {
    mid        = (high + low + 1) / 2;
    uint64_t p = primes[mid];
    if (p * p >= high_num) {
      high = mid - 1;
    } else {
      low = mid;
    }
  }
  if ((uint64_t)primes[low] * primes[low] >= high_num) {
    return 0; // No primes to iterate if they all are above high_num.
  } else {
    return low + 1; // We return the last known good index + 1 to make sure we get the right output.
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
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
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
static uint32_t *base_prime_sieve(uint64_t end, uint64_t *base_prime_count) {

  uint32_t count = 0; // making sure we're set to zero.
  uint64_t root  = isqrt(end);
  // Setup the bool array with all zeros
  unsigned char *is_composite = calloc(root + 1, 1);
  // Handle errors
  if (is_composite == NULL) {
    printf("Unable to allocate base sieve composites array. Exiting...\n");
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
        // We've found the last prime that still strikes within the base sieve.
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
  if (primes == NULL) {
    printf("Unable to allocate base primes array. Exiting...\n");
    free(is_composite);
    return NULL;
  }

  // And fill it.
  uint32_t index = 0;
  for (uint32_t i = 0; i <= root; i++) {
    if (!is_composite[i]) {
      primes[index++] = i;
    }
  }

  // We finished the base sieve setup, return and start the complete bool sieve.
  free(is_composite);
  *base_prime_count = count;
  return primes;
}

static uint64_t *build_pre_sieve(uint64_t nwords) {
  uint64_t *working_set = calloc(nwords, sizeof(uint64_t));
  if (working_set == NULL) {
    printf("Unable to allocate pre-sieve array. Exiting...\n");
    return NULL;
  }
  for (uint32_t i = 0; i < W_PRIME_COUNT; i++) {
    uint32_t p = W_PRIMES[i].prime;
    // We need to compute the segment relative offset for the otherwise global index in next_cursor
    uint64_t offset = (p - 1) / 2;
    // Now while we are below the size of the segment, we stride by p and mark each multiple.
    while (offset < nwords * 64) {
      working_set[offset >> 6] |= 1ULL << (offset & 63);
      offset += p;
    }
  }
  return working_set;
}

// Arcane Chicanery that needs a longer explainantion: we shouldn't check the wheel marked
// composites again. As such, by finding the open slots on the presieve, counting the gaps
// between them, and saving those gaps to a separate array, we can use these as multiples for the
// marking offset. This isn't enough however, so we also build the phases

// Builds the wheel_phase and wheel_gap arrays with the pre_sieve and base_primes arrays.
static uint32_t *build_wheel_gap(uint64_t *pre_sieve, uint32_t *live_count, uint32_t wheel_period,
                                 uint32_t *phase_by_wheel_offset) {
  // First we walk the inverted presieve one word and bit at a time.
  uint32_t  nwords             = (wheel_period + 63) / 64;
  uint32_t *wheel_gap          = calloc(wheel_period, sizeof(uint32_t));
  uint32_t *wheel_live_offsets = calloc(wheel_period, sizeof(uint32_t));
  for (uint32_t i = 0; i < nwords; i++) {
    uint64_t presieve_word = ~pre_sieve[i];
    if (i + 1 == nwords) {
      // If we're at the end our wheel period, we need to mask off the bits that are outside it.
      presieve_word &= (1ULL << (wheel_period & 63)) - 1;
    }
    if (presieve_word == 0ULL)
      continue; // If the word is 0, no primes, skip.
    while (presieve_word != 0) {
      unsigned bit = (unsigned)__builtin_ctzll(presieve_word); // Get the index of the first prime
      presieve_word &= presieve_word - 1;                      // Clear the last 1 for next check.
      uint64_t offset = i * 64 + bit;                          // We calculate up to segment offset.
      if (offset >= wheel_period) {
        continue;
      } else {
        wheel_live_offsets[*live_count] = offset;
        (*live_count)++;
      }
    }
  }

  for (uint32_t i = 1; i < *live_count; i++) {
    wheel_gap[i - 1] = wheel_live_offsets[i] - wheel_live_offsets[i - 1];
    phase_by_wheel_offset[wheel_live_offsets[i - 1]] = i - 1;
  }
  // And add the last gap, which is the gap between the last live offset and the first live offset.
  wheel_gap[*live_count - 1] =
      wheel_live_offsets[0] + wheel_period - wheel_live_offsets[*live_count - 1];
  phase_by_wheel_offset[wheel_live_offsets[*live_count - 1]] = *live_count - 1;
  free(wheel_live_offsets);
  return wheel_gap;
}

// Each prime walks the wheel gaps differently. We need to compute the 'phase' of those walks, by
// taking each prime 'p', and finding it's first strike within that mask such that
// ((p - 1) / 2 ) % 15015 = multiplier so that we can give the phase[multipler] => wheel_gap[phase]
// and it gives us exactly the next offset that
// 1. hasn't been marked by the pre-sieve, and
// 2. has factor p.

static uint32_t *build_wheel_phasing(uint32_t *base_primes, uint32_t base_prime_count,
                                     uint32_t wheel_period, uint32_t *phase_of_offsets) {
  // First we walk the inverted presieve one word and bit at a time.
  uint32_t *phase = calloc(base_prime_count, sizeof(uint32_t));
  for (uint32_t i = 0; i < base_prime_count; i++) {
    uint32_t p            = base_primes[i];
    uint32_t phase_offset = ((p - 1) / 2) % wheel_period;
    phase[i]              = phase_of_offsets[phase_offset];
  }
  return phase;
}

static uint64_t *build_rolling_cursor(uint32_t *primes, uint32_t base_prime_count) {
  uint64_t *cursor = malloc(base_prime_count * sizeof(uint64_t));
  for (uint32_t i = 0; i < base_prime_count; i++) {
    // Starting with p squared, as the first possible multiple.
    uint64_t p         = primes[i];
    uint64_t p_squared = p * p;

    // We map that square to the odds-only index
    cursor[i] = (p_squared - 1) / 2;
  }
  return cursor;
}

// Finds all primes from low to high with given composite array.
static void odds_seg_sieve(uint32_t *primes, uint64_t base_prime_count, uint32_t *wheel_gaps,
                           uint32_t live_count, uint32_t *prime_phase, uint64_t *is_composite,
                           uint64_t *next_cursor, uint64_t high_num, uint64_t low_num,
                           u128 *out_p_sum, uint64_t *out_p_count) {
  u128     segment_sum    = 0;
  uint64_t segment_count  = 0;
  uint64_t segment_size   = ((high_num - low_num + 1) / 2);
  uint64_t segment_base   = (low_num - 1) / 2;
  uint64_t segment_nwords = (segment_size + 63) / 64;
  uint32_t used_bits      = segment_size % 64;
  uint64_t tail_mask      = ~0ULL; // Starting with no tailmask
  if (segment_size % 64 != 0)
    tail_mask = (1ULL << used_bits) -
                1; // In case we get a ragged end, we need a mask to & off unused comps

  // If it's the first block, we set the weird composite and pre-sieve primes,
  // since the pre-sieve doesn't discriminate between residues and the initial primes:
  if (low_num == 1) {
    is_composite[0] |= 1ULL; // Thankfully, we don't have to do fancy tricks for this one.
    for (uint32_t i = 0; i < W_PRIME_COUNT; i++) {
      uint32_t p      = W_PRIMES[i].prime;
      uint64_t offset = (p - 1) / 2;
      is_composite[offset >> 6] &= ~(1ULL << (offset & 63));
    }
  }

  uint32_t active_prime_count = count_active_primes_binary(primes, base_prime_count, high_num);

  // Then we iterate through every prime in primes[] that has a multiple within the segment. Except
  // for the wheel primes, handled by the pre-sieve.
  for (uint64_t i = W_PRIME_COUNT + 1; i < active_prime_count; i++) {
    uint64_t p = primes[i];
    // We need to compute the segment relative offset for the otherwise global index in next_cursor
    uint64_t cursor = next_cursor[i];
    uint32_t offset = cursor - segment_base;
    // We also setup our phase for our prime;
    uint32_t phase = prime_phase[i];

    // Now while we are below the size of the segment, we stride by p and mark each multiple.
    while (offset < segment_size) {
      is_composite[offset >> 6] |= 1ULL << (offset & 63);
      // We use the phase to make sure we get to the right next wheel gap,
      offset += p * wheel_gaps[phase];
      phase++;
      // And we roll over to the first gap if we finish our list.
      if (phase == live_count)
        phase = 0;
    }
    // When we're done, we save the next multiple, adding back segment_base to get the global index
    next_cursor[i] = offset + segment_base;
    // And save our the prime's phase for next segment as well.
    prime_phase[i] = phase;
  }
  // Accumulate sum and prime count from the composite sieve.
  for (uint64_t i = 0; i < segment_nwords; i++) {
    uint64_t prime_word = ~is_composite[i]; // Invert the composite 'word' so primes are 1.
    if (i + 1 == segment_nwords)
      prime_word &= tail_mask; // If we're at the end of the segment, & the tailmask.
    if (prime_word == 0ULL)
      continue; // If the word is 0, no primes, skip.
    while (prime_word != 0) {
      unsigned bit = (unsigned)__builtin_ctzll(prime_word); // Get the index of the first prime
      prime_word &= prime_word - 1;                         // Clear the last 1 for next check.
      uint64_t offset = i * 64 + bit;                       // We calculate up to segment offset.
      segment_sum += (u128)(low_num + 2 * offset); // Finally sum the numeric value of that index.
      segment_count++;
    }
  }
  *out_p_count += segment_count;
  *out_p_sum += segment_sum;
}

// Sets up and manages the main sieve loop.
static int run_full_search(uint32_t *primes, uint64_t base_prime_count, uint64_t *pre_sieve,
                           uint32_t *wheel_gaps, uint32_t *prime_phase, uint32_t live_count,
                           uint32_t block_size, uint64_t end, u128 *prime_sum,
                           uint64_t *total_primes_counter, _Bool sum_only) {
  uint32_t last_update = 0;
  uint32_t nwords      = (block_size + 63) / 64;

  uint64_t *is_composite = calloc(nwords, (sizeof(uint64_t)));
  if (is_composite == NULL) {
    printf("Unable to allocate full sieve composites array. Exiting...\n");
    return EXIT_FAILURE;
  }
  memcpy(is_composite, pre_sieve, nwords * sizeof(*is_composite));
  // We initialize the rolling cursor array to eliminate a ton of our warm loop math.
  uint64_t *next_cursor = build_rolling_cursor(primes, base_prime_count);
  if (next_cursor == NULL) {
    printf("Unable to allocate full sieve cursor array. Exiting...\n");
    free(is_composite);
    return EXIT_FAILURE;
  }

  // Set up a loop to iterate by segment[i]
  for (uint32_t i = 0;; i++) {

    // Set the high and low numbers for the segment.
    uint64_t low  = ((i * (uint64_t)block_size) * 2) + 1;
    uint64_t high = (((i + 1) * (uint64_t)block_size) * 2) + 1;

    if (low > end) {
      // If the segment low number jumps over the end, we're done.
      break;
    }
    if (high > end) {
      high = end + 1;
    }

    odds_seg_sieve(primes, base_prime_count, wheel_gaps, live_count, prime_phase, is_composite,
                   next_cursor, high, low, prime_sum, total_primes_counter);
    // Reset for next cycle
    memcpy(is_composite, pre_sieve, nwords * sizeof(*is_composite));
    if (!sum_only)
      run_terminal_updates(high, end, &last_update, total_primes_counter);
  }
  free(next_cursor);
  free(is_composite);

  return EXIT_SUCCESS;
}

// Segmented Odds-only E-sieve with rolling cursors. Searches for primes between 2 and given arg or
// 2 and 2^31 with no arg. Hoping for ~50% better perf than non-odds only version on ends ~2^31.
int main(int argc, char *argv[]) {
  const char *endpoint_arg         = NULL;
  _Bool       sum_only             = false;
  uint64_t    end                  = 1;
  u128        prime_sum            = 2;                 // Adding 2 to start.
  uint64_t    total_primes_counter = 1;                 // Counting 2
  uint64_t    base_prime_count     = 0;                 // Primes in sqrt(end) base sieve array.
  uint32_t    wheel_period         = 15015;             // 3 * 5 * 7 * 11 * *13
  uint32_t    block_size           = wheel_period * 64; // Segment size (~120KiB of bits).
  uint64_t    nwords               = (block_size + 63) / 64;

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
    printf("Starting Base Search...\n");
  }
  // Late assign/alloc so we don't have to free on each possible error before now.

  // Now run the base sieve for the primes < sqrt(end)
  uint32_t *base_primes = base_prime_sieve(end, &base_prime_count);
  if (base_primes == NULL) {
    printf("Error in base primes search.\n");
    return EXIT_FAILURE;
  }

  if (!sum_only) {
    printf("Base Sieve complete.\n");
    printf("Base Prime Count is %lu\n", base_prime_count);
    printf("Starting pre-sieve pattern build...\n");
  }

  uint64_t *pre_sieve = build_pre_sieve(nwords);
  if (pre_sieve == NULL) {
    printf("Error in pre-sieve pattern build.\n");
    free(base_primes);
    return EXIT_FAILURE;
  }
  uint32_t *phase_of_offsets = calloc(wheel_period, sizeof(uint32_t));
  if (phase_of_offsets == NULL) {
    printf("Unable to allocate phase of offsets array. Exiting...\n");
    free(base_primes);
    free(pre_sieve);
    return EXIT_FAILURE;
  }
  uint32_t  live_count = 0;
  uint32_t *wheel_gaps = build_wheel_gap(pre_sieve, &live_count, wheel_period, phase_of_offsets);
  uint32_t *prime_phase =
      build_wheel_phasing(base_primes, base_prime_count, wheel_period, phase_of_offsets);

  if (!sum_only) {
    printf("Wheel Pattern complete.\n");
    printf("Starting final full-sieve...\n");
  }
  // If we didn't have any issues, run the real segmented sieve.
  if (run_full_search(base_primes, base_prime_count, pre_sieve, wheel_gaps, prime_phase, live_count,
                      block_size, end, &prime_sum, &total_primes_counter, sum_only) != 0) {
    free(base_primes);
    free(pre_sieve);
    return EXIT_FAILURE;
  }

  if (!sum_only && isatty(STDERR_FILENO)) {
    fprintf(stderr, "\rPrime count: %-10lu Completion %%: %3.2f \n", total_primes_counter, 100.00f);
    fflush(stderr);
  }

  if (!sum_only) {
    printf("Found %lu primes [0, %lu]\n", total_primes_counter, end);
    fputs("Sum of found primes is ", stdout);
    print_u128(stdout, prime_sum);
    putchar('\n');
  } else {
    print_u128(stdout, prime_sum);
    putchar('\n');
  }
  free(pre_sieve);
  free(phase_of_offsets);
  free(wheel_gaps);
  free(prime_phase);
  free(base_primes);
  return EXIT_SUCCESS;
}
