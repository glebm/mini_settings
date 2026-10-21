#ifndef _MINI_SETTINGS_LOCKED_FILE_H_
#define _MINI_SETTINGS_LOCKED_FILE_H_

#include <stdbool.h>
#include <stddef.h>

// A regular file held under an exclusive flock(2), replaced atomically via
// rename(2) so that concurrent readers never observe a partial write.
struct mini_settings_locked_file {
  int fd;
  char* path;  // Symlinks resolved. Owned.
};

// Returns true if `path` is a regular file or does not exist yet,
// i.e. if it can be locked and replaced.
bool mini_settings_is_replaceable_file(const char* path);

// Opens `path` (creating it if needed) and waits for an exclusive lock.
// On error, sets errno.
bool mini_settings_lock_file(const char* path,
                             struct mini_settings_locked_file* out);

// Reads the whole file. The result must be freed by the caller.
// On error, sets errno.
bool mini_settings_read_locked_file(struct mini_settings_locked_file* file,
                                    char** out_data, size_t* out_size);

// Atomically replaces the file contents with `data`, preserving the mode and,
// where permitted, the owner. On error, sets errno and leaves the file intact.
bool mini_settings_replace_locked_file(struct mini_settings_locked_file* file,
                                       const char* data, size_t size);

// Releases the lock.
void mini_settings_unlock_file(struct mini_settings_locked_file* file);

#endif  // _MINI_SETTINGS_LOCKED_FILE_H_
