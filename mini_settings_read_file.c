#include "mini_settings_read_file.h"

#include <stdio.h>
#include <stdlib.h>

bool mini_settings_try_read_file(const char *fname, char **out_data,
                                 size_t *out_size) {
  FILE *f = fopen(fname, "rb");

  if (f == NULL) return false;

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *data = malloc(fsize + 1);
  if (fread(data, fsize, 1, f) != 1 && ferror(f)) return false;

  fclose(f);
  data[fsize] = 0;

  *out_data = data;
  *out_size = fsize;
  return true;
}

void mini_settings_read_file(const char *fname, char **out_data,
                             size_t *out_size) {
  if (!mini_settings_try_read_file(fname, out_data, out_size)) {
    perror(fname);
    exit(EXIT_FAILURE);
  }
}
