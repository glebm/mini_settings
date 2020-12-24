#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_settings_read_file.h"
#include "mini_settings_set.h"

#ifndef DEFAULT_CONFIG_IN_PATH
#ifdef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_IN_PATH DEFAULT_CONFIG_PATH
#else
#define DEFAULT_CONFIG_IN_PATH "/dev/stdin"
#endif
#endif

#ifndef DEFAULT_CONFIG_OUT_PATH
#ifdef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_OUT_PATH DEFAULT_CONFIG_PATH
#else
#define DEFAULT_CONFIG_OUT_PATH "/dev/stdout"
#endif
#endif

static const char kUsage[] =
    "Usage: %s [-f CONFIG_FILE] <KEY> <VALUE> [KEY "
    "VALUE]...\n\n"
    "Sets value(s) in the config file.\n"
    "If a commented key exists, will uncomment it and set the value in-place"
    " instead of appending a new key\n\n"
    "By default, reads from " DEFAULT_CONFIG_IN_PATH
    " and writes to " DEFAULT_CONFIG_OUT_PATH "\n";

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, kUsage, argv[0]);
    return EXIT_FAILURE;
  }
  size_t kvs_size = argc - 1;
  const char **kvs = (const char **)argv + 1;
  const char *config_in_path = DEFAULT_CONFIG_IN_PATH;
  const char *config_out_path = DEFAULT_CONFIG_OUT_PATH;
  if (argv[1][0] == '-' && argv[1][1] == 'f') {
    if (argc < 3) {
      const char message[] = "-f requires an argument\n";
      fwrite(message, sizeof(char), sizeof(message), stderr);
      return EXIT_FAILURE;
    }
    config_in_path = config_out_path = argv[2];
    kvs += 2;
    kvs_size -= 2;
  }

  if (kvs_size == 0) {
    const char message[] = "-f requires an argument\n";
    fwrite(message, sizeof(char), sizeof(message), stderr);
    return EXIT_FAILURE;
  }

  if (kvs_size % 2 != 0) {
    const char message[] = "the number of key-value arguments must be even\n";
    fwrite(message, sizeof(char), sizeof(message), stderr);
    return EXIT_FAILURE;
  }

  char *config_contents;
  size_t config_contents_size;

  if (!mini_settings_try_read_file(config_in_path, &config_contents,
                                   &config_contents_size)) {
    if (errno == ENOENT) {
      config_contents = NULL;
      config_contents_size = 0;
    } else {
      perror(config_in_path);
      return EXIT_FAILURE;
    }
  }

  struct mini_settings_set_result_t result =
      mini_settings_set(config_contents, config_contents_size, kvs, kvs_size);
  if (result.error != NULL) {
    fwrite(result.error, sizeof(char), result.error_size, stderr);
    free(result.error);
    goto set_failed;
  }

  FILE *config_file = fopen(config_out_path, "w");
  if (config_file == NULL) {
    perror(config_out_path);
    goto set_failed;
  }
  fwrite(result.contents, sizeof(char), result.contents_size, config_file);
  if (ferror(config_file)) {
    perror("write error");
    goto set_failed;
  }
  fclose(config_file);

  free(result.contents);
  free(config_contents);
  return EXIT_SUCCESS;

set_failed:
  free(result.contents);
  free(config_contents);
  return EXIT_FAILURE;
}
