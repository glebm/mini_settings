# mini_settings

A fast C implementation of reading from and writing to a simple key-value text configuration file.

Provides 2 binaries to read/write a simple key-value config format.

## Build options

See `meson_options.txt`

Example:

```sh
meson setup builddir --buildtype=release \
  -Ddefault_config_path=/tmp/my.conf \
  -Dget_exe_name=my-get \
  -Dset_exe_name=my-set
```

### Concurrency

With `-Dconcurrency=true` (the default), `mini-settings-set -f CONFIG_FILE`
is safe to run concurrently with other `mini-settings-set` and
`mini-settings-get` calls on the same file:

* Writers take an exclusive `flock(2)` on the config file for the whole
  read-modify-write, so concurrent updates are never lost.
* The new contents are written to a temporary file in the same directory,
  `fsync`ed, and `rename`d over the config file, so readers always see either
  the old or the new file in full, and a crash never leaves a partial file.
  The file mode and (where permitted) owner are preserved, and symlinks are
  followed.

This requires write access to the config file's directory.
`flock(2)` may not be reliable on some network filesystems.

Pass `-Dconcurrency=false` to write the file in place instead.

### mini-settings-get

    Usage: mini-settings-get [-f CONFIG_FILE] <KEY> [KEY]...
    Prints the value of the key or returns a non-zero exit status.
    If multiple keys are given, tries them in order until it finds a key
     that exists.
    By default, reads from /dev/stdin

### mini-settings-set

    Usage: mini-settings-set [-f CONFIG_FILE] <KEY> <VALUE> [KEY VALUE]...
    Sets value(s) in the config file.
    If a commented key exists, will uncomment it and set the value in-place
     instead of appending a new key
    By default, reads from /dev/stdin and writes to /dev/stdout;
