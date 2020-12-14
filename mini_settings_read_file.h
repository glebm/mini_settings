#ifndef _MINI_SETTINGS_READ_FILE_H_
#define _MINI_SETTINGS_READ_FILE_H_

#include <stddef.h>

/* Reads the file. Exits on error. */
void mini_settings_read_file(const char *fname, char **out_data,
                             size_t *out_size);

#endif  // _MINI_SETTINGS_READ_FILE_H_
