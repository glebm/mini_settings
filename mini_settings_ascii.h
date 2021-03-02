#ifndef _MINI_SETTINGS_ASCII_H_
#define _MINI_SETTINGS_ASCII_H_

// Returns a pointer to the first non-leading whitespace.
// Only ' ' and '\t' are considered whitespace.
// Requires: begin <= end.
static inline const char *skip_leading_whitespace(const char *begin,
                                                  const char *end) {
  while (begin != end && (*begin == ' ' || *begin == '\t')) ++begin;
  return begin;
}

// Returns a pointer to the last non-whitespace.
// Only ' ' and '\t' are considered whitespace.
// Requires: begin <= end.
static inline const char *skip_trailing_whitespace(const char *begin,
                                                   const char *end) {
  while (begin != end && (*(end - 1) == ' ' || *(end - 1) == '\t')) --end;
  return end;
}

// Skips UTF-8 byte order mark.
// See https://en.wikipedia.org/wiki/Byte_order_mark
static inline const char *skip_utf8_bom(const char *begin, const char *end) {
  if (end - begin >= 3 && begin[0] == '\xEF' && begin[1] == '\xBB' &&
      begin[2] == '\xBF')
    return begin + 3;
  return begin;
}

#endif  // _MINI_SETTINGS_ASCII_H_
