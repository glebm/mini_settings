#define _DEFAULT_SOURCE

#include "mini_settings_locked_file.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mini_settings_read_file.h"

bool mini_settings_is_replaceable_file(const char* path) {
  struct stat st;
  if (stat(path, &st) != 0) return errno == ENOENT;
  return S_ISREG(st.st_mode);
}

bool mini_settings_lock_file(const char* path,
                             struct mini_settings_locked_file* out) {
  int fd;
  for (;;) {
    fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0666);
    if (fd < 0) return false;
    if (flock(fd, LOCK_EX) != 0) goto fail;

    // Another writer may have renamed a new file into place while we were
    // waiting for the lock, in which case we hold a lock on a stale inode.
    struct stat fd_st, path_st;
    if (fstat(fd, &fd_st) != 0) goto fail;
    char* resolved = realpath(path, NULL);
    if (resolved == NULL) {
      if (errno != ENOENT) goto fail;
    } else if (stat(resolved, &path_st) != 0) {
      free(resolved);
      if (errno != ENOENT) goto fail;
    } else if (fd_st.st_dev == path_st.st_dev &&
               fd_st.st_ino == path_st.st_ino) {
      out->fd = fd;
      out->path = resolved;
      return true;
    } else {
      free(resolved);
    }
    close(fd);
  }

fail:;
  const int saved_errno = errno;
  close(fd);
  errno = saved_errno;
  return false;
}

bool mini_settings_read_locked_file(struct mini_settings_locked_file* file,
                                    char** out_data, size_t* out_size) {
  return mini_settings_try_read_fd(file->fd, out_data, out_size);
}

static bool write_all(int fd, const char* data, size_t size) {
  while (size > 0) {
    const ssize_t written = write(fd, data, size);
    if (written < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    data += written;
    size -= written;
  }
  return true;
}

static void fsync_parent_dir(const char* path) {
  char* path_copy = strdup(path);
  if (path_copy == NULL) return;
  const int dir_fd = open(dirname(path_copy), O_RDONLY | O_DIRECTORY);
  free(path_copy);
  if (dir_fd < 0) return;
  fsync(dir_fd);
  close(dir_fd);
}

bool mini_settings_replace_locked_file(struct mini_settings_locked_file* file,
                                       const char* data, size_t size) {
  struct stat st;
  if (fstat(file->fd, &st) != 0) return false;

  // The temporary file must be on the same filesystem for rename to be atomic.
  static const char kSuffix[] = ".XXXXXX";
  const size_t path_len = strlen(file->path);
  char* tmp_path = malloc(path_len + sizeof(kSuffix));
  if (tmp_path == NULL) return false;
  memcpy(tmp_path, file->path, path_len);
  memcpy(tmp_path + path_len, kSuffix, sizeof(kSuffix));

  int tmp_fd = mkstemp(tmp_path);
  if (tmp_fd < 0) {
    free(tmp_path);
    return false;
  }

  if (!write_all(tmp_fd, data, size)) goto fail;
  if (fchmod(tmp_fd, st.st_mode & 07777) != 0) goto fail;
  // Changing the owner requires privileges; keep ours if not permitted.
  if (st.st_uid != geteuid() || st.st_gid != getegid()) {
    (void)fchown(tmp_fd, st.st_uid, st.st_gid);
  }
  if (fsync(tmp_fd) != 0) goto fail;
  const int close_result = close(tmp_fd);
  tmp_fd = -1;
  if (close_result != 0) goto fail;
  if (rename(tmp_path, file->path) != 0) goto fail;
  free(tmp_path);
  fsync_parent_dir(file->path);
  return true;

fail:;
  const int saved_errno = errno;
  if (tmp_fd >= 0) close(tmp_fd);
  unlink(tmp_path);
  free(tmp_path);
  errno = saved_errno;
  return false;
}

void mini_settings_unlock_file(struct mini_settings_locked_file* file) {
  close(file->fd);
  free(file->path);
  file->fd = -1;
  file->path = NULL;
}
