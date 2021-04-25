#ifndef _MINI_SETTINGS_SET_H_
#define _MINI_SETTINGS_SET_H_

#include <stdbool.h>
#include <stddef.h>

struct mini_settings_set_result_t {
  char *contents;  // must be freed by the caller
  size_t contents_size;

  // Owned if set and must be freed by the caller.
  char *error;
  size_t error_size;
};

struct mini_settings_set_result_t mini_settings_set(const char *config_contents,
                                                    size_t config_contents_size,
                                                    const char *kvs[],
                                                    size_t kvs_size,
                                                    bool validate);

#endif  // _MINI_SETTINGS_SET_H_
