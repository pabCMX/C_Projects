// AI-GENERATED REFERENCE EXAMPLE — do not treat this as your chapter exercise work.
//
// This is one possible solution for the token-cost calculator project.
// Build your own version in aiTokenCostCalculator.c, using LESSON_token_cost.html
// as a guide. Peek here only when stuck.
//
// Build (example only):
//   gcc -std=c23 -Wall -Wextra -pedantic -g aiTokenCostCalculator.example.c
//     -o build/aiTokenCostCalculator.example.exe
//
// Usage:
//   ./build/aiTokenCostCalculator.example.exe <csv-path> [year] [month]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LINE_MAX 8192
#define FIELD_MAX 32
#define NAME_MAX 64
#define MODEL_MAX 16

typedef struct {
  double input_no_cache;
  double input_cache_write;
  double input_cache_read;
  double output;
  long   long_context_threshold; // 0 means no long-context tier
  double long_input_no_cache;
  double long_input_cache_write;
  double long_input_cache_read;
  double long_output;
} Pricing;

typedef struct {
  char    name[NAME_MAX];
  char    api_label[NAME_MAX];
  Pricing rates;
} ModelPricing;

typedef struct {
  char   name[NAME_MAX];
  int    events;
  long   cache_write;
  long   no_cache;
  long   cache_read;
  long   output;
  int    long_context_events;
  double cost;
} ModelStats;

static const ModelPricing MODEL_PRICING[] = {
    {
        .name      = "composer-2.5-fast",
        .api_label = "Cursor Composer 2.5 Fast",
        .rates =
            {
                .input_no_cache    = 3.00,
                .input_cache_write = 3.00,
                .input_cache_read  = 0.50,
                .output            = 15.00,
            },
    },
    {
        .name      = "gpt-5.5-medium",
        .api_label = "OpenAI gpt-5.5",
        .rates =
            {
                .input_no_cache         = 3.00,
                .input_cache_write      = 3.00,
                .input_cache_read       = 0.50,
                .output                 = 15.00,
                .long_context_threshold = 272000L,
                .long_input_no_cache    = 10.00,
                .long_input_cache_write = 10.00,
                .long_input_cache_read  = 1.00,
                .long_output            = 45.00,
            },
    },
    {
        .name      = "claude-fable-5-thinking-medium",
        .api_label = "Anthropic Claude Fable 5",
        .rates =
            {
                .input_no_cache    = 10.00,
                .input_cache_write = 12.50,
                .input_cache_read  = 1.00,
                .output            = 50.00,
            },
    },
    {
        .name      = "claude-opus-4-8",
        .api_label = "Anthropic Claude Opus 4.8",
        .rates =
            {
                .input_no_cache    = 5.00,
                .input_cache_write = 6.25,
                .input_cache_read  = 0.50,
                .output            = 25.00,
            },
    },
    {
        .name      = "claude-opus-4-8-thinking-high",
        .api_label = "Anthropic Claude Opus 4.8 (high thinking)",
        .rates =
            {
                .input_no_cache    = 5.00,
                .input_cache_write = 6.25,
                .input_cache_read  = 0.50,
                .output            = 25.00,
            },
    },
    {
        .name      = "claude-opus-4-8-thinking-medium",
        .api_label = "Anthropic Claude Opus 4.8 (medium thinking)",
        .rates =
            {
                .input_no_cache    = 5.00,
                .input_cache_write = 6.25,
                .input_cache_read  = 0.50,
                .output            = 25.00,
            },
    },
    {
        .name      = "claude-opus-4-8-thinking-low",
        .api_label = "Anthropic Claude Opus 4.8 (low thinking)",
        .rates =
            {
                .input_no_cache    = 5.00,
                .input_cache_write = 6.25,
                .input_cache_read  = 0.50,
                .output            = 25.00,
            },
    },
    {
        .name      = "claude-opus-4-8-fast",
        .api_label = "Anthropic Claude Opus 4.8 Fast mode",
        .rates =
            {
                .input_no_cache    = 10.00,
                .input_cache_write = 12.50,
                .input_cache_read  = 1.00,
                .output            = 50.00,
            },
    },
};

static void trim_quotes(char *field) {
  size_t len = strlen(field);
  if (len >= 2 && field[0] == '"' && field[len - 1] == '"') {
    memmove(field, field + 1, len - 2);
    field[len - 2] = '\0';
  }
}

static int split_csv_line(char *line, char *fields[], int max_fields) {
  int   count  = 0;
  char *cursor = line;

  while (*cursor != '\0' && count < max_fields) {
    fields[count++] = cursor;

    if (*cursor == '"') {
      fields[count - 1] = ++cursor;
      while (*cursor != '\0') {
        if (*cursor == '"') {
          if (*(cursor + 1) == '"') {
            memmove(cursor, cursor + 1, strlen(cursor));
            cursor++;
          } else {
            *cursor = '\0';
            cursor++;
            if (*cursor == ',') {
              cursor++;
            }
            break;
          }
        } else {
          cursor++;
        }
      }
      continue;
    }

    while (*cursor != '\0' && *cursor != ',') {
      cursor++;
    }

    if (*cursor == ',') {
      *cursor = '\0';
      cursor++;
    }
  }

  return count;
}

static long parse_token_count(const char *field) {
  char   buffer[64];
  size_t i = 0;

  while (field[i] != '\0' && i + 1 < sizeof(buffer)) {
    buffer[i] = field[i];
    i++;
  }
  buffer[i] = '\0';
  trim_quotes(buffer);

  if (buffer[0] == '\0') {
    return 0L;
  }

  return strtol(buffer, NULL, 10);
}

static int parse_iso_date(const char *field, int *year, int *month, int *day) {
  char buffer[64];
  strncpy(buffer, field, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';
  trim_quotes(buffer);

  return sscanf(buffer, "%d-%d-%d", year, month, day) == 3;
}

static const ModelPricing *find_pricing(const char *model_name) {
  for (size_t i = 0; i < sizeof(MODEL_PRICING) / sizeof(MODEL_PRICING[0]); i++) {
    if (strcmp(model_name, MODEL_PRICING[i].name) == 0) {
      return &MODEL_PRICING[i];
    }
  }
  return NULL;
}

static ModelStats *find_or_add_stats(ModelStats stats[], int *stats_count, const char *name) {
  for (int i = 0; i < *stats_count; i++) {
    if (strcmp(stats[i].name, name) == 0) {
      return &stats[i];
    }
  }

  if (*stats_count >= MODEL_MAX) {
    return NULL;
  }

  ModelStats *slot = &stats[*stats_count];
  strncpy(slot->name, name, sizeof(slot->name) - 1);
  slot->name[sizeof(slot->name) - 1] = '\0';
  (*stats_count)++;
  return slot;
}

static int uses_long_context(const Pricing *rates, long cache_write, long no_cache,
                             long cache_read) {
  if (rates->long_context_threshold <= 0L) {
    return 0;
  }

  long input_tokens = cache_write + no_cache + cache_read;
  return input_tokens > rates->long_context_threshold;
}

static double event_cost(const Pricing *rates, long cache_write, long no_cache, long cache_read,
                         long output, int *long_context_flag) {
  if (uses_long_context(rates, cache_write, no_cache, cache_read)) {
    *long_context_flag = 1;
    return (cache_write * rates->long_input_cache_write + no_cache * rates->long_input_no_cache +
            cache_read * rates->long_input_cache_read + output * rates->long_output) /
           1000000.0;
  }

  *long_context_flag = 0;
  return (cache_write * rates->input_cache_write + no_cache * rates->input_no_cache +
          cache_read * rates->input_cache_read + output * rates->output) /
         1000000.0;
}

static void print_usage(const char *program_name) {
  printf("Usage: %s <csv-path> [year] [month]\n", program_name);
  printf("\n");
  printf("Examples:\n");
  printf("  %s ~/Downloads/usage-events-2026-06-18.csv 2026 6\n", program_name);
  printf("  %s ~/Downloads/usage-events-2026-06-18.csv\n", program_name);
}

static void get_current_year_month(int *year, int *month) {
  time_t    now = time(NULL);
  struct tm local_time;

#if defined(_WIN32)
  localtime_s(&local_time, &now);
#else
  localtime_r(&now, &local_time);
#endif

  *year  = local_time.tm_year + 1900;
  *month = local_time.tm_mon + 1;
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
  if (argc < 2 || argc > 4) {
    print_usage(argv[0]);
    return 1;
  }

  const char *csv_path = argv[1];
  int         filter_year;
  int         filter_month;

  if (argc == 4) {
    filter_year  = atoi(argv[2]);
    filter_month = atoi(argv[3]);
  } else if (argc == 2) {
    get_current_year_month(&filter_year, &filter_month);
  } else {
    print_usage(argv[0]);
    return 1;
  }

  if (filter_month < 1 || filter_month > 12) {
    fprintf(stderr, "Error: month must be between 1 and 12.\n");
    return 1;
  }

  FILE *input = fopen(csv_path, "r");
  if (input == NULL) {
    fprintf(stderr, "Error: could not open %s\n", csv_path);
    return 1;
  }

  char line[LINE_MAX];
  if (fgets(line, sizeof(line), input) == NULL) {
    fprintf(stderr, "Error: CSV file is empty.\n");
    fclose(input);
    return 1;
  }

  ModelStats stats[MODEL_MAX];
  int        stats_count    = 0;
  int        matched_events = 0;
  int        unknown_models = 0;
  double     total_cost     = 0.0;

  while (fgets(line, sizeof(line), input) != NULL) {
    char *fields[FIELD_MAX];
    int   field_count = split_csv_line(line, fields, FIELD_MAX);

    if (field_count < 10) {
      continue;
    }

    for (int i = 0; i < field_count; i++) {
      trim_quotes(fields[i]);
    }

    int year  = 0;
    int month = 0;
    int day   = 0;
    if (!parse_iso_date(fields[0], &year, &month, &day)) {
      continue;
    }

    if (year != filter_year || month != filter_month) {
      continue;
    }

    const char         *model_name = fields[4];
    const ModelPricing *pricing    = find_pricing(model_name);
    if (pricing == NULL) {
      unknown_models++;
      continue;
    }

    long cache_write = parse_token_count(fields[6]);
    long no_cache    = parse_token_count(fields[7]);
    long cache_read  = parse_token_count(fields[8]);
    long output      = parse_token_count(fields[9]);

    int    long_context = 0;
    double cost =
        event_cost(&pricing->rates, cache_write, no_cache, cache_read, output, &long_context);

    ModelStats *model_stats = find_or_add_stats(stats, &stats_count, model_name);
    if (model_stats == NULL) {
      fprintf(stderr, "Error: too many distinct models (max %d).\n", MODEL_MAX);
      fclose(input);
      return 1;
    }

    model_stats->events++;
    model_stats->cache_write += cache_write;
    model_stats->no_cache += no_cache;
    model_stats->cache_read += cache_read;
    model_stats->output += output;
    model_stats->cost += cost;
    if (long_context) {
      model_stats->long_context_events++;
    }

    matched_events++;
    total_cost += cost;
  }

  fclose(input);

  qsort(stats, (size_t)stats_count, sizeof(ModelStats), compare_stats_by_cost);

  printf("Token cost estimate for %d-%02d\n", filter_year, filter_month);
  printf("Source: %s\n", csv_path);
  printf("Matched events: %d\n", matched_events);
  if (unknown_models > 0) {
    printf("Skipped unknown models: %d events\n", unknown_models);
  }
  printf("\n");

  if (matched_events == 0) {
    printf("No matching usage events found.\n");
    return 0;
  }

  printf("Total API-equivalent cost: $%.2f\n\n", total_cost);

  printf("%-34s %7s %11s %11s %9s %10s %7s\n", "Model", "Events", "In (no c)", "Cache read",
         "Output", "Cost", "Share");
  printf("%-34s %7s %11s %11s %9s %10s %7s\n", "----------------------------------", "-------",
         "-----------", "-----------", "---------", "----------", "-------");

  for (int i = 0; i < stats_count; i++) {
    const ModelStats   *row     = &stats[i];
    const ModelPricing *pricing = find_pricing(row->name);
    double              share   = total_cost > 0.0 ? (row->cost / total_cost) * 100.0 : 0.0;

    char no_cache_buf[16];
    char cache_read_buf[16];
    char output_buf[16];

    snprintf(no_cache_buf, sizeof(no_cache_buf), "%.2fM", row->no_cache / 1000000.0);
    snprintf(cache_read_buf, sizeof(cache_read_buf), "%.2fM", row->cache_read / 1000000.0);
    snprintf(output_buf, sizeof(output_buf), "%.2fM", row->output / 1000000.0);

    printf("%-34s %7d %11s %11s %9s $%8.2f %6.1f%%", row->name, row->events, no_cache_buf,
           cache_read_buf, output_buf, row->cost, share);

    if (pricing != NULL && row->long_context_events > 0) {
      printf("  (%d long-context)", row->long_context_events);
    }
    printf("\n");
  }

  printf("\nPricing notes:\n");
  for (size_t i = 0; i < sizeof(MODEL_PRICING) / sizeof(MODEL_PRICING[0]); i++) {
    const ModelPricing *entry = &MODEL_PRICING[i];
    int                 seen  = 0;

    for (int j = 0; j < stats_count; j++) {
      if (strcmp(stats[j].name, entry->name) == 0) {
        seen = 1;
        break;
      }
    }

    if (!seen) {
      continue;
    }

    printf("- %s (%s): $%.2f/M in, $%.2f/M cache write, $%.2f/M cache read, $%.2f/M out",
           entry->name, entry->api_label, entry->rates.input_no_cache,
           entry->rates.input_cache_write, entry->rates.input_cache_read, entry->rates.output);

    if (entry->rates.long_context_threshold > 0L) {
      printf("; long context above %ld input tokens uses 2x input and 1.5x output rates",
             entry->rates.long_context_threshold);
    }
    printf("\n");
  }

  return 0;
}
