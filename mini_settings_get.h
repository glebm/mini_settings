#ifndef _MINI_SETTINGS_GET_H_
#define _MINI_SETTINGS_GET_H_

#include <stdbool.h>
#include <stddef.h>

struct mini_settings_get_result_t {
  const char *key;
  size_t key_size;
  const char *value;
  size_t value_size;

  // Owned if set and must be freed by the caller.
  char *error;
  size_t error_size;
};

struct mini_settings_get_result_t mini_settings_get(const char *config_contents,
                                                    size_t config_size,
                                                    const char *keys[],
                                                    size_t keys_size,
                                                    bool validate);

#endif  // _MINI_SETTINGS_GET_H_
