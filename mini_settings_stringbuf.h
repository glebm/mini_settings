#ifndef _MINI_SETTINGS_STRINGBUF_H_
#define _MINI_SETTINGS_STRINGBUF_H_

#include <stddef.h>

struct mini_settings_stringbuf {
  char *data;
  size_t size;
  size_t capacity;
};

void mini_settings_stringbuf_init(struct mini_settings_stringbuf *buf,
                                  size_t capacity);

void mini_settings_stringbuf_append(struct mini_settings_stringbuf *buf,
                                    const char *str, size_t size);

void mini_settings_stringbuf_append_line(struct mini_settings_stringbuf *buf,
                                         const char *str, size_t size);

void mini_settings_stringbuf_append_char(struct mini_settings_stringbuf *buf,
                                         char c);

#endif  // _MINI_SETTINGS_STRINGBUF_H_
