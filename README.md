# mini_settings

A fast C implementation of reading from and writing to a simple key-value text configuration file.

Provides 2 binaries to read/write a simple key-value config format.

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

The default config file location for the binaries can be changed via the `DEFAULT_CONFIG_PATH` define.
