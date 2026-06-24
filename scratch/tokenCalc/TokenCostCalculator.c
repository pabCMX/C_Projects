#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  double input_no_cache;
  double input_cache_write;
  double input_cache_read;
  double output;
  double long_context_threshold;
  double long_input_no_cache;
  double long_input_cache_write;
  double long_input_cache_read;
  double long_output;
} PricingRates;

typedef struct {
  char         name[64];
  PricingRates rates;
} ModelPricing;

typedef struct {
  char   name[64];
  int    events;
  long   no_cache;
  long   cache_write;
  long   cache_read;
  long   output;
  double cost;
} ModelStats;

typedef struct {
  char suffix[64];
} Suffixes;

static const ModelPricing MODEL_PRICING[] = {
    {.name  = "composer-2.5",
     .rates = {.input_no_cache    = 0.50,
               .input_cache_write = 0.50,
               .input_cache_read  = 0.20,
               .output            = 2.50}},
    {.name  = "composer-2.5-fast",
     .rates = {.input_no_cache    = 3.00,
               .input_cache_write = 3.00,
               .input_cache_read  = 0.50,
               .output            = 15.00}},
    {.name  = "composer-2",
     .rates = {.input_no_cache    = 0.50,
               .input_cache_write = 0.50,
               .input_cache_read  = 0.20,
               .output            = 2.50}},
    {.name  = "composer-2-fast",
     .rates = {.input_no_cache    = 1.50,
               .input_cache_write = 1.50,
               .input_cache_read  = 0.35,
               .output            = 7.50}},
    {.name  = "composer-1.5",
     .rates = {.input_no_cache    = 3.50,
               .input_cache_write = 3.50,
               .input_cache_read  = 0.35,
               .output            = 17.50}},
    {.name  = "composer-1",
     .rates = {.input_no_cache    = 1.25,
               .input_cache_write = 1.25,
               .input_cache_read  = 0.125,
               .output            = 10.00}},
    {.name  = "gpt-5.5",
     .rates = {.input_no_cache         = 3.00,
               .input_cache_write      = 3.00,
               .input_cache_read       = 0.50,
               .output                 = 15.00,
               .long_context_threshold = 272000,
               .long_input_no_cache    = 10.00,
               .long_input_cache_write = 10.00,
               .long_input_cache_read  = 1.00,
               .long_output            = 45.00}},
    {.name  = "gpt-5.5-fast",
     .rates = {.input_no_cache    = 12.50,
               .input_cache_write = 12.50,
               .input_cache_read  = 1.25,
               .output            = 75.00}},
    {.name  = "gpt-5.4",
     .rates = {.input_no_cache    = 2.5,
               .input_cache_write = 2.5,
               .input_cache_read  = 0.25,
               .output            = 15.00}},
    {.name  = "gpt-5.3-codex",
     .rates = {.input_no_cache    = 1.75,
               .input_cache_write = 1.75,
               .input_cache_read  = 0.175,
               .output            = 14.00}},
    {.name  = "gpt-5.3-codex-fast",
     .rates = {.input_no_cache    = 3.50,
               .input_cache_write = 3.50,
               .input_cache_read  = 0.35,
               .output            = 28.00}},
    {.name  = "gpt-5.2",
     .rates = {.input_no_cache    = 1.75,
               .input_cache_write = 1.75,
               .input_cache_read  = 0.175,
               .output            = 14.00}},
    {.name  = "gpt-5.2-fast",
     .rates = {.input_no_cache    = 3.50,
               .input_cache_write = 3.50,
               .input_cache_read  = 0.35,
               .output            = 28.00}},
    {.name  = "gpt-5.2-codex",
     .rates = {.input_no_cache    = 1.75,
               .input_cache_write = 1.75,
               .input_cache_read  = 0.175,
               .output            = 14.00}},
    {.name  = "gpt-5.1-codex-max",
     .rates = {.input_no_cache    = 1.25,
               .input_cache_write = 1.25,
               .input_cache_read  = 0.125,
               .output            = 10.00}},
    {.name  = "gpt-5.1-codex-mini",
     .rates = {.input_no_cache    = 0.25,
               .input_cache_write = 0.25,
               .input_cache_read  = 0.025,
               .output            = 2.00}},
    {.name  = "claude-opus-4-8",
     .rates = {.input_no_cache    = 5.00,
               .input_cache_write = 6.25,
               .input_cache_read  = 0.50,
               .output            = 25.00}},
    {.name  = "claude-opus-4-8-fast",
     .rates = {.input_no_cache    = 10.00,
               .input_cache_write = 12.50,
               .input_cache_read  = 1.00,
               .output            = 50.00}},
    {.name  = "claude-4.6-opus",
     .rates = {.input_no_cache    = 5.00,
               .input_cache_write = 6.25,
               .input_cache_read  = 0.50,
               .output            = 25.00}},
    {.name  = "claude-4.6-opus-fast",
     .rates = {.input_no_cache    = 30.00,
               .input_cache_write = 37.50,
               .input_cache_read  = 3.00,
               .output            = 150.00}},
    {.name  = "claude-4.5-opus",
     .rates = {.input_no_cache    = 5.00,
               .input_cache_write = 6.25,
               .input_cache_read  = 0.50,
               .output            = 25.00}},
    {.name  = "claude-4.6-sonnet",
     .rates = {.input_no_cache    = 3.00,
               .input_cache_write = 3.75,
               .input_cache_read  = 0.30,
               .output            = 15.00}},
    {.name  = "claude-4.5-sonnet",
     .rates = {.input_no_cache    = 3.00,
               .input_cache_write = 3.75,
               .input_cache_read  = 0.30,
               .output            = 15.00}},
    {.name  = "claude-fable-5",
     .rates = {.input_no_cache    = 10.00,
               .input_cache_write = 12.50,
               .input_cache_read  = 1.00,
               .output            = 50.00}},
    {.name  = "gemini-3-pro-preview",
     .rates = {.input_no_cache    = 2.00,
               .input_cache_write = 2.00,
               .input_cache_read  = 0.20,
               .output            = 12.00}},
    {.name  = "kimi-k2.5",
     .rates = {.input_no_cache    = 0.60,
               .input_cache_write = 0.60,
               .input_cache_read  = 0.10,
               .output            = 3.00}},
    {.name  = "auto",
     .rates = {.input_no_cache    = 0.50,
               .input_cache_write = 0.50,
               .input_cache_read  = 0.20,
               .output            = 2.50}},
};

static const Suffixes REASONING_SUFFIXES[] = {{.suffix = "-xhigh"},  {.suffix = "-high"},
                                              {.suffix = "-medium"}, {.suffix = "-low"},
                                              {.suffix = "-none"},   {.suffix = "-thinking"}};

static const char ERRORED_FIELD[32] = "Errored, No Charge";

static const int FALLBACK_MODEL_INDEX = 26;

#define MAX_MODELS 32

static long parse_field_as_long(char *field) {
  return strtol(field, NULL, 10);
}

static const ModelPricing *find_pricing(const char *model_name) {
  // Creating a working copy of the model name to avoid modifying the original.

  char working_name[64];
  strncpy(working_name, model_name, sizeof(working_name) - 1);
  working_name[sizeof(working_name) - 1] = '\0';
  size_t model_count                     = sizeof(MODEL_PRICING) / sizeof(MODEL_PRICING[0]);
  size_t working_name_length             = strlen(working_name);
  int    is_fast                         = 0;
  int    no_more_suffixes                = 0;

  // First we try to match a straight model name exactly.

  for (size_t i = 0; i < model_count; i++) {
    if (strcmp(working_name, MODEL_PRICING[i].name) == 0) {
      // If we find it directly, just return that.

      return &MODEL_PRICING[i];
    }
  }

  // Otherwise we attempt to strip everything besides the name starting with "-fast".

  if (working_name_length >= 5 && strcmp(working_name + working_name_length - 5, "-fast") == 0) {
    is_fast                               = 1;
    working_name[working_name_length - 5] = '\0';
  }

  // Now onto 'thinking-[tier] and -[tier]' suffixes.

  for (int pass = 0; pass < 2; pass++) {
    // First we find the last hyphen in the working name.

    char *last_hyphen = strrchr(working_name, '-');
    // First we check if we found a hyphen. No hyphen means no more suffixes.
    if (last_hyphen == NULL) {
      no_more_suffixes = 1;
      break;
    }
    // If we found a hyphen, we set no_more_suffixes to 1 to prep to break the loop.
    no_more_suffixes = 1;
    // Then we iterate through the list of suffixes one at a time.
    for (size_t i = 0; i < sizeof(REASONING_SUFFIXES) / sizeof(REASONING_SUFFIXES[0]); i++) {
      // If we find one, just set the end at the hyphen and break to the while loop.
      if (strcmp(last_hyphen, REASONING_SUFFIXES[i].suffix) == 0) {
        *last_hyphen     = '\0';
        no_more_suffixes = 0;
        break;
      }
    }
    // If we didn't find a suffix, we break the loop early.
    if (no_more_suffixes) {
      break;
    }
  }

  // Now we can match the working name to the list of models.
  if (is_fast) {
    strcat(working_name, "-fast");
  }
  // and we check with the exact same logic as the first name check.
  for (size_t i = 0; i < model_count; i++) {
    if (strcmp(working_name, MODEL_PRICING[i].name) == 0) {
      // If we find it directly, just return that.
      return &MODEL_PRICING[i];
    }
  }
  // If we don't find it, we return the auto pricing.
  return &MODEL_PRICING[FALLBACK_MODEL_INDEX];
}

static double calculate_row_cost(char *fields[], const ModelPricing *pricing) {
  // First check the model pricing table size and init cost variable.
  long       no_cache    = parse_field_as_long(fields[7]);
  long       cache_read  = parse_field_as_long(fields[8]);
  long       cache_write = parse_field_as_long(fields[6]);
  long       output      = parse_field_as_long(fields[9]);
  const long Mil         = 1'000'000;
  double     cost        = 0.0;

  // If we find it we check if we're over the long context threshold.
  if (pricing->rates.long_context_threshold > 0.0 &&
      no_cache + cache_read + cache_write >= pricing->rates.long_context_threshold) {
    // if so we use the long costs.
    cost += (no_cache * pricing->rates.long_input_no_cache) / Mil;
    cost += (cache_read * pricing->rates.long_input_cache_read) / Mil;
    cost += (cache_write * pricing->rates.long_input_cache_write) / Mil;
    return cost += (output * pricing->rates.long_output) / Mil;
  } else {
    // Otherwise we use normal costs.
    cost += (no_cache * pricing->rates.input_no_cache) / Mil;
    cost += (cache_read * pricing->rates.input_cache_read) / Mil;
    cost += (cache_write * pricing->rates.input_cache_write) / Mil;
    return cost += (output * pricing->rates.output) / Mil;
  }
}

static int accumulate_stats_from_row(ModelStats *stats, char *fields[], int *stats_count) {
  // We create a found index to set data.
  int found = -1;

  // First check if we have the model name already in our list.
  for (int i = 0; i < *stats_count; i++) {
    if (strcmp(stats[i].name, fields[4]) == 0) {
      // If a model name matches, we set the found index to the current stat index.
      found = i;
      // Then exit the stats checking loop.
      break;
    }
  }

  // If not, add it
  if (found == -1) {
    // We'll set found to the current stats max index.
    found = *stats_count;
    // Then update the max stats index.
    (*stats_count)++;
    // We copy the name directly from the field into the stats (making space for the final char).
    strncpy(stats[found].name, fields[4], sizeof(stats[found].name) - 1);
    // Then we add a null terminator.
    stats[found].name[sizeof(stats[found].name) - 1] = '\0';
  }

  // Now we accumulate each of the model's statistics based on the current fields into the stats
  // struct totals.
  stats[found].events++;
  stats[found].no_cache += parse_field_as_long(fields[7]);
  stats[found].cache_read += parse_field_as_long(fields[8]);
  stats[found].cache_write += parse_field_as_long(fields[6]);
  stats[found].output += parse_field_as_long(fields[9]);
  // Before we calculate cost, we need to find the model's pricing rates, and give them to the
  // cost calc.
  const ModelPricing *pricing = find_pricing(fields[4]);
  stats[found].cost += calculate_row_cost(fields, pricing);

  return 0;
}

static int split_csv_line(char *line, char *fields[], int max_fields) {
  int   count  = 0;
  char *cursor = line;

  while (*cursor != '\0' && count < max_fields) {
    // First we skip any newline cruft by scanning past it.
    while (*cursor == '\n' || *cursor == '\r')
      cursor++;
    // Then we check if we're at the end of the line; break since we're done.
    if (*cursor == '\0')
      break;
    // Now we assign field[x] = cursor *position* (not value) then increment count to x+1
    fields[count++] = cursor;

    // Check for a starting " character to skip
    if (*cursor == '"') {
      // Step back to the just assigned field, and assign it to the next position forward.
      fields[count - 1] = ++cursor;
      // Note that we also move ourselves forward at the same time.

      // Now we scan ahead until we either hit the line-end char or another "
      while (*cursor != '\0' && *cursor != '"') {
        cursor++;
      }

      // If we hit another ", replace it with a line-end char
      if (*cursor == '"') {
        *cursor = '\0';
        cursor++;
      }
    } else {
      // if we didn't hit a " char, we scan for either line-end, new-line, or ','
      while (*cursor != '\0' && *cursor != ',' && *cursor != '\n') {
        cursor++;
      }
    }
    // Once we hit either , or new-line; replace with line-end
    if (*cursor == ',' || *cursor == '\n') {
      *cursor = '\0';
      cursor++;
    }
  }

  return count;
}

static int compare_stats_by_cost(const void *left, const void *right) {
  const ModelStats *a = left;
  const ModelStats *b = right;
  if (a->cost < b->cost) {
    return 1;
  }
  if (a->cost > b->cost) {
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("Missing args. Usage: %s <csv-path>\n", argv[0]);
    return 1;
  }

  char      *path = argv[1];
  char       line[8192];
  char      *fields[32];
  int        field_count       = 0;
  ModelStats stats[MAX_MODELS] = {0};
  int        stats_count       = 0;
  double     total_cost        = 0;
  long       total_no_cache    = 0;
  long       total_cache_write = 0;
  long       total_cache_read  = 0;
  long       total_output      = 0;

  // File opening with the filename path as first arg, and mode in second.
  //  r = reading (file must exist),
  //  w = writing (creates new or truncaes existing to zero length),
  //  a = append to end, creates if !file, r+ = read and writing (file must exist)
  //  w+ = read and write but creates if !file, a+ = read and append.
  //  add b to any mode for binary files.
  FILE *input = fopen(path, "r");
  if (input == NULL) {
    printf("Error opening file %s, exiting...\n", path);
    return 1;
  }

  if (fgets(line, sizeof(line), input) == NULL) {
    printf("File was empty\n");
    return 1;
  }

  while (fgets(line, sizeof(line), input) != NULL) {
    field_count = split_csv_line(line, fields, 32);
    if (field_count != 12) {
      printf("Error reading line, bad field number:");
      printf("%s", line);
      continue;
    }
    // First check if field[3] is errored or not.
    if (strcmp(fields[3], ERRORED_FIELD) == 0) {
      // Just skip it.
      continue;
    }
    // We start with the stats struct, all the current fields, and a pointer to the total stats
    // index
    accumulate_stats_from_row(stats, fields, &stats_count);
  }
  fclose(input);

  // Now we sort the stats by cost.
  qsort(stats, (size_t)stats_count, sizeof(ModelStats), compare_stats_by_cost);

  // Now we print the stats starting with the header.
  printf("%-32s %12s %12s %12s %12s %12s\n", "Model", "Input Tks", "Cache Writes", "Cache Read",
         "Output", "Cost");
  for (int i = 0; i < stats_count; i++) {
    total_cost += stats[i].cost;
    total_no_cache += stats[i].no_cache;
    total_cache_write += stats[i].cache_write;
    total_cache_read += stats[i].cache_read;
    total_output += stats[i].output;
    printf("%-32s %12ld %12ld %12ld %12ld %12.2f\n", stats[i].name, stats[i].no_cache,
           stats[i].cache_write, stats[i].cache_read, stats[i].output, stats[i].cost);
  }
  printf("%-32s %12ld %12ld %12ld %12ld %12.2f\n", "Total", total_no_cache, total_cache_write,
         total_cache_read, total_output, total_cost);

  return 0;
}
