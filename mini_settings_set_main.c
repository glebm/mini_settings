#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_settings_read_file.h"
#include "mini_settings_set.h"

#if MINI_SETTINGS_CONCURRENCY
#include "mini_settings_locked_file.h"
#endif

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
    "Usage: %s [-f CONFIG_FILE] [--validate] <KEY> <VALUE> [KEY "
    "VALUE]...\n\n"
    "Sets value(s) in the config file.\n"
    "If a commented key exists, will uncomment it and set the value in-place"
    " instead of appending a new key\n\n"
    "If --validate is passed, checks that the file is in the key=value"
    " format.\n\n"
    "By default, reads from " DEFAULT_CONFIG_IN_PATH
    " and writes to " DEFAULT_CONFIG_OUT_PATH "\n";

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, kUsage, argv[0]);
    return EXIT_FAILURE;
  }
  bool validate = false;
  const char *config_in_path = DEFAULT_CONFIG_IN_PATH;
  const char *config_out_path = DEFAULT_CONFIG_OUT_PATH;
  unsigned i = 1;
  for (; i < argc && argv[i][0] == '-'; ++i) {
    if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        const char message[] = "-f requires an argument\n";
        fwrite(message, sizeof(char), sizeof(message), stderr);
        return EXIT_FAILURE;
      }
      config_in_path = config_out_path = argv[++i];
    } else if (strcmp(argv[i], "--validate") == 0) {
      validate = true;
    } else if (strcmp(argv[i], "--") == 0) {
      ++i;
      break;
    } else {
      fprintf(stderr, "unknown argument: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }
  const char **kvs = (const char **)argv + i;
  const size_t kvs_size = argc - i;

  if (kvs_size == 0) {
    const char message[] = "Pass at least 1 key and 1 value\n";
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

#if MINI_SETTINGS_CONCURRENCY
  // Serialize concurrent writers and replace the file atomically so that
  // readers never see a partially written file.
  struct mini_settings_locked_file locked_file = {.fd = -1};
  const bool use_locked_file =
      strcmp(config_in_path, config_out_path) == 0 &&
      mini_settings_is_replaceable_file(config_out_path);
  if (use_locked_file) {
    if (!mini_settings_lock_file(config_out_path, &locked_file) ||
        !mini_settings_read_locked_file(&locked_file, &config_contents,
                                        &config_contents_size)) {
      perror(config_out_path);
      if (locked_file.fd >= 0) mini_settings_unlock_file(&locked_file);
      return EXIT_FAILURE;
    }
  }
#else
  const bool use_locked_file = false;
#endif
  if (!use_locked_file &&
      !mini_settings_try_read_file(config_in_path, &config_contents,
                                   &config_contents_size)) {
    if (errno == ENOENT) {
      config_contents = NULL;
      config_contents_size = 0;
    } else {
      perror(config_in_path);
      return EXIT_FAILURE;
    }
  }

  struct mini_settings_set_result_t result = mini_settings_set(
      config_contents, config_contents_size, kvs, kvs_size, validate);
  if (result.error != NULL) {
    fwrite(result.error, sizeof(char), result.error_size, stderr);
    free(result.error);
    fputc('\n', stderr);
    goto set_failed;
  }

#if MINI_SETTINGS_CONCURRENCY
  if (use_locked_file) {
    if (!mini_settings_replace_locked_file(&locked_file, result.contents,
                                           result.contents_size)) {
      perror(config_out_path);
      goto set_failed;
    }
    mini_settings_unlock_file(&locked_file);
    free(result.contents);
    free(config_contents);
    return EXIT_SUCCESS;
  }
#endif

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
#if MINI_SETTINGS_CONCURRENCY
  if (use_locked_file) mini_settings_unlock_file(&locked_file);
#endif
  free(result.contents);
  free(config_contents);
  return EXIT_FAILURE;
}
