#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_settings_get.h"
#include "mini_settings_read_file.h"

#ifndef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_PATH "/dev/stdin"
#endif

static const char kUsage[] =
    "Usage: %s [-f CONFIG_FILE] <KEY> [KEY]...\n\n"
    "Prints the value of the key or returns a non-zero exit status.\n"
    "If multiple keys are given, tries them in order until it finds a key"
    " that exists.\n\n"
    "By default, reads from " DEFAULT_CONFIG_PATH "\n";

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, kUsage, argv[0]);
    return EXIT_FAILURE;
  }
  const char *config_path = DEFAULT_CONFIG_PATH;
  unsigned i = 1;
  for (; i < argc && argv[i][0] == '-'; ++i) {
    if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        const char message[] = "-f requires an argument\n";
        fwrite(message, sizeof(char), sizeof(message), stderr);
        return EXIT_FAILURE;
      }
      config_path = argv[++i];
    } else if (strcmp(argv[i], "--") == 0) {
      ++i;
      break;
    } else {
      fprintf(stderr, "unknown argument: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }
  const size_t num_keys = argc - i;
  const char **keys = (const char **)argv + i;

  if (num_keys == 0) {
    const char message[] = "at least 1 key argument is required\n";
    fwrite(message, sizeof(char), sizeof(message), stderr);
    return EXIT_FAILURE;
  }

  char *config_contents;
  size_t config_contents_size;
  mini_settings_read_file(config_path, &config_contents, &config_contents_size);

  struct mini_settings_get_result_t result =
      mini_settings_get(config_contents, config_contents_size, keys, num_keys);
  if (result.error != NULL) {
    fwrite(result.error, sizeof(char), result.error_size, stderr);
    free(result.error);
    fputc('\n', stderr);
    goto get_failed;
  } else if (result.value == NULL) {
    // Key not found.
    goto get_failed;
  }

  fwrite(result.value, sizeof(char), result.value_size, stdout);
  fputc('\n', stdout);
  if (ferror(stdout)) goto get_failed;

  free(config_contents);
  return EXIT_SUCCESS;

get_failed:
  free(config_contents);
  return EXIT_FAILURE;
}
