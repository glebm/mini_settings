#ifndef _MINI_SETTINGS_READ_FILE_H_
#define _MINI_SETTINGS_READ_FILE_H_

#include <stdbool.h>
#include <stddef.h>

// Reads from the file descriptor until EOF. Works with pipes.
// On error, sets errno.
bool mini_settings_try_read_fd(int fd, char** out_data, size_t* out_size);

// Reads the file. On error, sets errno.
bool mini_settings_try_read_file(const char *fname, char **out_data,
                                 size_t *out_size);

// Reads the file. Exits on error.
void mini_settings_read_file(const char *fname, char **out_data,
                             size_t *out_size);

#endif  // _MINI_SETTINGS_READ_FILE_H_
