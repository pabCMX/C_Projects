#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
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
} Pricing;

typedef struct {
  char    name[64];
  Pricing rates;
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

static const ModelPricing MODEL_PRICING[] = {
    {.name  = "composer-2.5-fast",
     .rates = {.input_no_cache    = 3.00,
               .input_cache_write = 3.00,
               .input_cache_read  = 0.50,
               .output            = 15.00}},
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
    {.name  = "claude-4.6-sonnet",
     .rates = {.input_no_cache    = 3.00,
               .input_cache_write = 3.75,
               .input_cache_read  = 0.30,
               .output            = 15.00}},
    {.name  = "claude-fable-5",
     .rates = {.input_no_cache    = 10.00,
               .input_cache_write = 12.50,
               .input_cache_read  = 1.00,
               .output            = 50.00}},
    {.name  = "auto",
     .rates = {.input_no_cache    = 0.50,
               .input_cache_write = 0.50,
               .input_cache_read  = 0.20,
               .output            = 2.50}},
};

int Add_to_stats(ModelStats *stats, char *fields[], int modelCount) {
  int stats_count = 0;
  int found       = -1;

  // First check if we have the model name already in our list.
  for (int i = 0; i < stats_count; i++) {
    if (strcmp(stats[i].name, fields[4]) == 0) {
      found = i;
      break;
    }
  }

  // If not, add it
  if (found =)
}

int split_csv_line(char *line, char *fields[], int max_fields) {
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

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("Missing args. Usage: %s <csv-path>\n", argv[0]);
    return 1;
  }

  char      *path = argv[1];
  char       line[8192];
  char      *fields[32];
  int        fieldCount = 0;
  const int  modelCount = sizeof(MODEL_PRICING) / sizeof(MODEL_PRICING[0]);
  ModelStats stats[8]   = {0};

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
  printf("%-32s %12s %12s %12s %12s %12s\n", "Model", "Input Tks", "Cache Writes", "Cache Read",
         "Output", "Cost");

  while (fgets(line, sizeof(line), input) != NULL) {
    fieldCount = split_csv_line(line, fields, 32);
    if (fieldCount != 12) {
      printf("Error reading line, bad field number:");
      printf("%s", line);
    }

    Add_to_stats(stats, fields, modelCount);
    printf("%-32s %12s %12s %12s %12s\n", fields[4], fields[7], fields[6], fields[8], fields[9]);
  }

  printf("Total Models with Pricing: %d\n", modelCount);
  fclose(input);

  return 0;
}
