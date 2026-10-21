#define _DEFAULT_SOURCE

#include "mini_settings_read_file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

bool mini_settings_try_read_fd(int fd, char** out_data, size_t* out_size) {
  struct stat st;
  if (fstat(fd, &st) != 0) return false;

  // The size is only a hint: it is unknown for pipes and the file may change.
  // Reserve room for the terminating NUL and 1 more byte so that EOF of a file
  // whose size is known is detected without reallocating.
  size_t capacity = (S_ISREG(st.st_mode) ? st.st_size : 4096) + 2;
  size_t size = 0;
  char* data = malloc(capacity);
  if (data == NULL) return false;
  for (;;) {
    if (size + 1 == capacity) {
      capacity *= 2;
      char* new_data = realloc(data, capacity);
      if (new_data == NULL) {
        free(data);
        errno = ENOMEM;
        return false;
      }
      data = new_data;
    }
    const ssize_t num_read = read(fd, data + size, capacity - size - 1);
    if (num_read == 0) break;
    if (num_read < 0) {
      if (errno == EINTR) continue;
      const int saved_errno = errno;
      free(data);
      errno = saved_errno;
      return false;
    }
    size += num_read;
  }
  data[size] = 0;

  *out_data = data;
  *out_size = size;
  return true;
}

bool mini_settings_try_read_file(const char* fname, char** out_data,
                                 size_t* out_size) {
  const int fd = open(fname, O_RDONLY | O_CLOEXEC);
  if (fd < 0) return false;
  const bool ok = mini_settings_try_read_fd(fd, out_data, out_size);
  const int saved_errno = errno;
  close(fd);
  errno = saved_errno;
  return ok;
}

void mini_settings_read_file(const char *fname, char **out_data,
                             size_t *out_size) {
  if (!mini_settings_try_read_file(fname, out_data, out_size)) {
    perror(fname);
    exit(EXIT_FAILURE);
  }
}
