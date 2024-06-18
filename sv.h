#ifndef _SV_H_
#define _SV_H_

#ifdef __cplusplus
  #include <cstdint>
#else
  #include <stdint.h>
#endif

#define SV_NPOS (sv_index_t)(-1)

#define svl(cstr_literal) sv_create(cstr_literal, sizeof(cstr_literal) - 1)
#define svc(cstr)         sv_create_from_cstr(cstr)
#define sv_empty          svl("")
#define sv_fmt            "%.*s"
#define sv_arg(sv)        (int)sv.length, sv.data

typedef unsigned long int sv_index_t;

typedef struct cstring_view {
  const char* data;
  sv_index_t length;
} cstring_view;

typedef int (*sv_for_split_callback)(cstring_view, void*);

#ifdef __cplusplus
extern "C" {
#endif

cstring_view sv_create(const char* data, sv_index_t length);
cstring_view sv_create_from_cstr(const char* data);
const char* sv_begin(cstring_view sv);
const char* sv_end(cstring_view sv);
char sv_at(cstring_view sv, sv_index_t index);
char sv_front(cstring_view sv);
char sv_back(cstring_view sv);
int sv_is_empty(cstring_view sv);
cstring_view sv_remove_prefix(cstring_view sv, sv_index_t n);
cstring_view sv_remove_suffix(cstring_view sv, sv_index_t n);
void sv_swap(cstring_view sv1, cstring_view sv2);
cstring_view sv_substr(cstring_view sv, sv_index_t pos, sv_index_t count);
int sv_compare(cstring_view sv1, cstring_view sv2);
int sv_starts_with(cstring_view sv1, cstring_view sv2);
int sv_starts_with_insensitive(cstring_view sv1, cstring_view sv2);

int sv_starts_with_char(cstring_view sv1, char c);
int sv_ends_with(cstring_view sv1, cstring_view sv2);
int sv_ends_with_char(cstring_view sv1, char c);
int sv_contains(cstring_view sv1, cstring_view sv2);
int sv_contains_insensitive(cstring_view sv1, cstring_view sv2);

int sv_count_char(cstring_view sv, char c);

sv_index_t sv_find_char(cstring_view sv1, char c, sv_index_t pos);
sv_index_t sv_find(cstring_view sv1, cstring_view sv2, sv_index_t pos);

sv_index_t sv_find_insensitive(cstring_view sv1, cstring_view sv2, sv_index_t pos);

sv_index_t sv_rfind_char(cstring_view sv1, char c, sv_index_t pos);
sv_index_t sv_rfind(cstring_view sv1, cstring_view sv2, sv_index_t pos);

sv_index_t sv_find_first_of_char(cstring_view sv, char c, sv_index_t pos);
sv_index_t sv_find_first_of(cstring_view sv1, cstring_view sv2, sv_index_t pos);

// same as sv_find_first_of, but return the char found and stores the index in
// index pointer. returns '\0' when not found
char sv_find_first_of_switch(cstring_view sv1, cstring_view sv2, sv_index_t pos, sv_index_t* index);

int sv_is_alpha(char c);
int sv_is_numeric(char c);
int sv_is_alphanum(char c);
char sv_tolower(char c);
char sv_toupper(char c);

sv_index_t sv_find_last_of_char(cstring_view sv, char c, sv_index_t pos);
sv_index_t sv_find_last_of(cstring_view sv1, cstring_view sv2, sv_index_t pos);
sv_index_t sv_find_first_not_of_char(cstring_view sv, char c, sv_index_t pos);
sv_index_t sv_find_first_not_of(cstring_view sv1, cstring_view sv2, sv_index_t pos);
sv_index_t sv_find_last_not_of_char(cstring_view sv, char c, sv_index_t pos);
sv_index_t sv_find_last_not_of(cstring_view sv1, cstring_view sv2, sv_index_t pos);

cstring_view sv_consume_until_first_of(cstring_view sv1, cstring_view sv2);
cstring_view sv_consume_until_first_not_of(cstring_view sv1, cstring_view sv2);
cstring_view sv_consume_until_last_of(cstring_view sv1, cstring_view sv2);
cstring_view sv_consume_until_last_not_of(cstring_view sv1, cstring_view sv2);

cstring_view sv_split_next(cstring_view input, cstring_view delim, cstring_view* token);
cstring_view sv_split_next_quote(cstring_view input, cstring_view delim, char open_quote, char close_quote, cstring_view* token);
int sv_for_split(cstring_view input, cstring_view delim, sv_for_split_callback callback, void* arg);

int sv_parse_int(cstring_view sv, int* value);
int sv_parse_uint64(cstring_view sv, uint64_t* value);
int sv_parse_uint32(cstring_view sv, uint32_t* value);
int sv_parse_uint8(cstring_view sv, uint8_t* value);
int sv_parse_float(cstring_view sv, float* value);
char* sv_strdup(cstring_view sv);
int sv_read_file(const char* filename, cstring_view* sv);
int sv_read_file_stdin(cstring_view* sv);
void sv_read_file_free(cstring_view sv);

#define SV_FOR_SPLIT(token, input, delim)                            \
  for (cstring_view token, sv = sv_split_next(input, delim, &token); \
       !sv_is_empty(sv) || !sv_is_empty(token);                      \
       sv = sv_split_next(sv, delim, &token))

#define SV_FOR_SPLIT_QUOTE(token, input, open, close, delim)                            \
  for (cstring_view token, sv = sv_split_next_quote(input, delim, open, close, &token); \
       !sv_is_empty(sv) || !sv_is_empty(token);                                         \
       sv = sv_split_next_quote(sv, delim, open, close, &token))

#ifdef __cplusplus
}
#endif

#ifdef SV_IMPLEMENTATION

  #ifdef __cplusplus
    #include <climits>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
  #else
    #include <limits.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
  #endif

cstring_view sv_create(const char* data, sv_index_t length) {
  cstring_view sv = {
      .data   = data,
      .length = length,
  };
  return sv;
}

cstring_view sv_create_from_cstr(const char* data) {
  return sv_create(data, strlen(data));
}

const char* sv_begin(cstring_view sv) { return sv.data; }

const char* sv_end(cstring_view sv) { return sv.data + (sv.length - 1); }

char sv_at(cstring_view sv, sv_index_t index) { return sv.data[index]; }

char sv_front(cstring_view sv) { return sv_begin(sv)[0]; }

char sv_back(cstring_view sv) { return sv_end(sv)[0]; }

int sv_is_empty(cstring_view sv) { return sv.length == 0; }

cstring_view sv_remove_prefix(cstring_view sv, sv_index_t n) {
  return sv_create(sv.data + n, sv.length - n);
}

cstring_view sv_remove_suffix(cstring_view sv, sv_index_t n) {
  return sv_create(sv.data, sv.length - n);
}

void sv_swap(cstring_view sv1, cstring_view sv2) {
  cstring_view tmp = sv1;
  sv1              = sv2;
  sv2              = tmp;
}

cstring_view sv_substr(cstring_view sv, sv_index_t pos, sv_index_t count) {
  if (count == SV_NPOS) count = sv.length;
  return sv_create(sv.data + pos, count < sv.length - pos ? count : sv.length - pos);
}

int sv_compare(cstring_view sv1, cstring_view sv2) {
  if (sv1.length != sv2.length) return 0;

  for (int i = 0; i < (int)sv1.length; i++) {
    if (sv1.data[i] != sv2.data[i]) return 0;
  }

  return 1;
}

int sv_starts_with_char(cstring_view sv1, char c) {
  return !sv_is_empty(sv1) && sv_front(sv1) == c;
}

int sv_starts_with(cstring_view sv1, cstring_view sv2) {
  if (sv2.length > sv1.length) return 0;

  for (int i = 0; i < (int)sv2.length; i++) {
    if (sv1.data[i] != sv2.data[i]) return 0;
  }

  return 1;
}

int sv_starts_with_insensitive(cstring_view sv1, cstring_view sv2) {
  if (sv2.length > sv1.length) return 0;

  for (int i = 0; i < (int)sv2.length; i++) {
    if (sv_tolower(sv1.data[i]) != sv_tolower(sv2.data[i])) return 0;
  }

  return 1;
}

int sv_ends_with_char(cstring_view sv1, char c) {
  return !sv_is_empty(sv1) && sv_back(sv1) == c;
}

int sv_ends_with(cstring_view sv1, cstring_view sv2) {
  if (sv2.length > sv1.length) return 0;

  for (int i = 0; i < (int)sv2.length; i++) {
    if ((sv_end(sv1) - sv2.length + 1)[i] != sv2.data[i]) return 0;
  }

  return 1;
}

int sv_contains(cstring_view sv1, cstring_view sv2) {
  return sv_find(sv1, sv2, 0) != SV_NPOS;
}

int sv_contains_insensitive(cstring_view sv1, cstring_view sv2) {
  return sv_find_insensitive(sv1, sv2, 0) != SV_NPOS;
}

int sv_count_char(cstring_view sv, char c) {
  int count = 0;
  for (int i = 0; i < (int)sv.length; i++) {
    if (sv.data[i] == c) count++;
  }
  return count;
}

sv_index_t sv_find_char(cstring_view sv1, char c, sv_index_t pos) {
  if (sv_is_empty(sv1)) return SV_NPOS;
  if (pos == SV_NPOS) pos = 0;

  for (int i = pos; i < (int)sv1.length; i++) {
    if (sv1.data[i] == c) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) && sv_is_empty(sv1)) return 0;
  if (sv2.length > sv1.length) return SV_NPOS;

  if (pos == SV_NPOS) pos = 0;

  for (int i = pos; i < (int)sv1.length; i++) {
    cstring_view v = sv_substr(sv1, i, sv2.length);
    if (sv_starts_with(v, sv2)) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find_insensitive(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) && sv_is_empty(sv1)) return 0;
  if (sv2.length > sv1.length) return SV_NPOS;
  if (pos == SV_NPOS) pos = 0;

  for (int i = pos; i < (int)sv1.length; i++) {
    cstring_view v = sv_substr(sv1, i, sv2.length);
    if (sv_starts_with_insensitive(v, sv2)) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_rfind_char(cstring_view sv1, char c, sv_index_t pos) {
  if (sv_is_empty(sv1)) return SV_NPOS;

  if (pos == SV_NPOS) pos = sv1.length - 1;

  for (int i = pos; i >= 0; i--) {
    if (sv1.data[i] == c) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_rfind(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) && sv_is_empty(sv1)) return 0;
  if (sv2.length > sv1.length) return SV_NPOS;

  if (pos == SV_NPOS) pos = sv1.length - 1;

  for (int i = pos; i >= 0; i--) {
    cstring_view v = sv_substr(sv1, i, SV_NPOS);
    if (sv_starts_with(v, sv2)) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find_first_of_char(cstring_view sv, char c, sv_index_t pos) {
  return sv_find_char(sv, c, pos);
}

sv_index_t sv_find_first_of(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) || sv_is_empty(sv1)) return SV_NPOS;

  for (int i = pos; i < (int)sv1.length; i++) {
    for (int j = 0; j < (int)sv2.length; j++) {
      if (sv1.data[i] == sv2.data[j]) return i;
    }
  }

  return SV_NPOS;
}

char sv_find_first_of_switch(cstring_view sv1, cstring_view sv2, sv_index_t pos, sv_index_t* index) {
  if (sv_is_empty(sv2) || sv_is_empty(sv1)) return '\0';

  for (int i = pos; i < (int)sv1.length; i++) {
    for (int j = 0; j < (int)sv2.length; j++) {
      if (sv1.data[i] == sv2.data[j]) {
        if (index != NULL) *index = i;
        return sv1.data[i];
      }
    }
  }

  if (index != NULL) *index = SV_NPOS;
  return '\0';
}

int sv_is_alpha(char c) {
  if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
    return 1;
  }
  return 0;
}

int sv_is_numeric(char c) {
  if ('0' <= c && c <= '9') {
    return 1;
  }
  return 0;
}

int sv_is_alphanum(char c) {
  if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
    return 1;
  }
  return 0;
}

char sv_tolower(char c) {
  if ('A' <= c && c <= 'Z') {
    return c + 'a' - 'A';
  }
  return c;
}

char sv_toupper(char c) {
  if ('a' <= c && c <= 'z') {
    return c - 'a' + 'A';
  }
  return c;
}

sv_index_t sv_find_last_of_char(cstring_view sv, char c, sv_index_t pos) {
  return sv_rfind_char(sv, c, pos);
}

sv_index_t sv_find_last_of(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) || sv_is_empty(sv1)) return SV_NPOS;

  if (pos == SV_NPOS) pos = sv1.length - 1;

  for (int i = pos; i >= 0; i--) {
    for (int j = 0; j < (int)sv2.length; j++) {
      if (sv1.data[i] == sv2.data[j]) return i;
    }
  }

  return SV_NPOS;
}

sv_index_t sv_find_first_not_of_char(cstring_view sv, char c, sv_index_t pos) {
  if (sv_is_empty(sv)) return SV_NPOS;

  for (int i = pos; i < (int)sv.length; i++) {
    if (sv.data[i] != c) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find_first_not_of(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv1) && sv_is_empty(sv2)) return 0;

  if (sv_is_empty(sv2)) return SV_NPOS;

  for (int i = pos; i < (int)sv1.length; i++) {
    int j;
    for (j = 0; j < (int)sv2.length; j++) {
      if (sv1.data[i] == sv2.data[j]) break;
    }
    if (j == (int)sv2.length) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find_last_not_of_char(cstring_view sv, char c, sv_index_t pos) {
  if (sv_is_empty(sv)) return SV_NPOS;

  if (pos == SV_NPOS) pos = sv.length - 1;

  for (int i = pos; i >= 0; i--) {
    if (sv.data[i] != c) return i;
  }

  return SV_NPOS;
}

sv_index_t sv_find_last_not_of(cstring_view sv1, cstring_view sv2, sv_index_t pos) {
  if (sv_is_empty(sv2) && sv_is_empty(sv1)) return SV_NPOS;
  if (pos == 0) return SV_NPOS;

  if (pos == SV_NPOS) pos = sv1.length - 1;

  if (sv_is_empty(sv2)) return pos;

  for (int i = pos; i >= 0; i--) {
    int j;
    for (j = 0; j < (int)sv2.length; j++) {
      if (sv1.data[i] == sv2.data[j]) break;
    }
    if (j == (int)sv2.length) return i;
  }

  return SV_NPOS;
}

cstring_view sv_consume_until_first_of(cstring_view sv1, cstring_view sv2) {
  sv_index_t index = sv_find_first_of(sv1, sv2, 0);
  if (index == SV_NPOS) return sv_empty;
  return sv_remove_prefix(sv1, index);
}

cstring_view sv_consume_until_first_not_of(cstring_view sv1, cstring_view sv2) {
  sv_index_t index = sv_find_first_not_of(sv1, sv2, 0);
  if (index == SV_NPOS) return sv_empty;
  return sv_remove_prefix(sv1, index);
}

cstring_view sv_consume_until_last_of(cstring_view sv1, cstring_view sv2) {
  sv_index_t index = sv_find_last_of(sv1, sv2, SV_NPOS);
  if (index == SV_NPOS) return sv_empty;
  return sv_remove_suffix(sv1, sv1.length - index - 1);
}

cstring_view sv_consume_until_last_not_of(cstring_view sv1, cstring_view sv2) {
  sv_index_t index = sv_find_last_not_of(sv1, sv2, SV_NPOS);
  if (index == SV_NPOS) return sv_empty;
  return sv_remove_suffix(sv1, sv1.length - index - 1);
}

cstring_view sv_split_next(cstring_view input, cstring_view delim, cstring_view* token) {
  // trim left delims
  input = sv_consume_until_first_not_of(input, delim);

  // input was all delims
  if (sv_is_empty(input)) {
    *token = sv_empty;
    return sv_empty;
  }

  sv_index_t pos = sv_find_first_of(input, delim, 0);

  // no delim found
  if (pos == SV_NPOS) {
    *token = input;
    return sv_empty;
  }

  *token = sv_substr(input, 0, pos);
  return sv_remove_prefix(input, token->length);
}

cstring_view sv_split_next_quote(cstring_view input, cstring_view delim, char open_quote, char close_quote, cstring_view* token) {
  // trim left delims
  input = sv_consume_until_first_not_of(input, delim);

  // input was all delims
  if (sv_is_empty(input)) {
    *token = sv_empty;
    return sv_empty;
  }

  if (sv_starts_with_char(input, open_quote)) {
    sv_index_t pos = sv_find_first_of_char(sv_remove_prefix(input, 1), close_quote, 0);

    // empty quoted string
    // will return token with included open close quotes,
    // so the user can detect if it is empty
    if (pos == 0) {
      *token = sv_substr(input, 0, 2);
      return sv_remove_prefix(input, token->length);
    }

    // fallback to keyword
    if (pos == SV_NPOS) {
      sv_index_t pos = sv_find_first_of(input, delim, 0);

      if (pos == SV_NPOS) {
        *token = input;
        return sv_empty;
      }

      *token = sv_substr(input, 0, pos);
      return sv_remove_prefix(input, token->length);
    }

    *token = sv_substr(sv_remove_prefix(input, 1), 0, pos);
    return sv_remove_prefix(sv_remove_prefix(input, 1), token->length + 1); // + 1 for end quote
  } else {
    sv_index_t pos = sv_find_first_of(input, delim, 0);

    // no delim found
    if (pos == SV_NPOS) {
      *token = input;
      return sv_empty;
    }

    *token = sv_substr(input, 0, pos);
    return sv_remove_prefix(input, token->length);
  }
}

int sv_for_split(cstring_view input, cstring_view delim, sv_for_split_callback callback, void* arg) {
  SV_FOR_SPLIT(token, input, delim) {
    int ret = callback(token, arg);
    if (!ret) return ret;
  }
  return 1;
}

int sv_parse_int(cstring_view sv, int* value) {
  if (sv_is_empty(sv)) return 0;
  if (sv.length > 11) // An int can hold up to 10 digits (+1 for potential '-' sign)
    return 0;

  int negative = 0;

  if (sv_front(sv) == '-') {
    negative = 1;
    sv       = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
  }

  if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;

  int tmp = 0;

  // Parse with overflow checking if the number has 10 digits
  if (sv.length == 10) {
    int limit       = (negative ? -(INT_MIN / 10) : INT_MAX / 10);
    int limit_digit = (negative ? -(INT_MIN % 10) : INT_MAX % 10);

    size_t i = 0;

    // Parse without overflow checking until the 9th digit.
    for (; i < sv.length && i < 9; ++i) {
      tmp = tmp * 10 + (sv.data[i] - '0');
    }

    // Parse the remaining digits with overflow checking.
    for (; i < sv.length; ++i) {
      int digit = sv.data[i] - '0';

      // Check for overflow.
      if (tmp > limit || (tmp == limit && digit > limit_digit))
        return 0; // Overflow would occur if we added this digit.

      tmp = tmp * 10 + digit;
    }
  } else { // No overflow possible
    for (size_t i = 0; i < sv.length; ++i) {
      tmp = tmp * 10 + (sv_at(sv, i) - '0');
    }
  }

  *value = negative ? -tmp : tmp;

  return 1;
}

int sv_parse_uint64(cstring_view sv, uint64_t* value) {
  if (sv_is_empty(sv)) return 0;
  if (sv.length > 20) // A uint64_t can hold up to 20 digits.
    return 0;

  if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;

  uint64_t tmp = 0;

  // Parse with overflow checking if the number has 20 digits
  if (sv.length == 20) {
    size_t i = 0;

    // Parse without overflow checking until the 19th digit.
    for (; i < sv.length && i < 19; ++i) {
      tmp = tmp * 10 + (sv.data[i] - '0');
    }

    // Parse the remaining digits with overflow checking.
    for (; i < sv.length; ++i) {
      uint64_t limit       = ((uint64_t)-1 / 10);
      uint64_t limit_digit = ((uint64_t)-1 % 10);
      uint64_t digit       = sv.data[i] - '0';

      // Check for overflow.
      if (tmp > limit || (tmp == limit && digit > limit_digit))
        return 0; // Overflow would occur if we added this digit.

      tmp = tmp * 10 + digit;
    }
  } else { // No overflow possible
    for (size_t i = 0; i < sv.length; ++i) {
      tmp = tmp * 10 + (sv_at(sv, i) - '0');
    }
  }

  *value = tmp;

  return 1;
}

int sv_parse_uint32(cstring_view sv, uint32_t* value) {
  if (sv_is_empty(sv)) return 0;
  if (sv.length > 10) // A uint64_t can hold up to 10 digits.
    return 0;

  if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;

  uint32_t tmp = 0;

  // Parse with overflow checking if the number has 10 digits
  if (sv.length == 10) {
    size_t i = 0;

    // Parse without overflow checking until the 19th digit.
    for (; i < sv.length && i < 9; ++i) {
      tmp = tmp * 10 + (sv.data[i] - '0');
    }

    // Parse the remaining digits with overflow checking.
    for (; i < sv.length; ++i) {
      uint32_t limit       = ((uint32_t)-1 / 10);
      uint32_t limit_digit = ((uint32_t)-1 % 10);
      uint32_t digit       = sv.data[i] - '0';

      // Check for overflow.
      if (tmp > limit || (tmp == limit && digit > limit_digit))
        return 0; // Overflow would occur if we added this digit.

      tmp = tmp * 10 + digit;
    }
  } else { // No overflow possible
    for (size_t i = 0; i < sv.length; ++i) {
      tmp = tmp * 10 + (sv_at(sv, i) - '0');
    }
  }

  *value = tmp;

  return 1;
}

int sv_parse_uint8(cstring_view sv, uint8_t* value) {
  if (sv_is_empty(sv)) return 0;
  if (sv.length > 3) // A uint8_t can hold up to 3 digits.
    return 0;

  if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;

  uint8_t tmp = 0;

  // Parse with overflow checking if the number has 3 digits
  if (sv.length == 3) {
    size_t i = 0;

    // Parse without overflow checking until the 2nd digit.
    for (; i < sv.length && i < 2; ++i) {
      tmp = tmp * 10 + (sv.data[i] - '0');
    }

    // Parse the remaining digits with overflow checking.
    for (; i < sv.length; ++i) {
      uint8_t limit       = ((uint8_t)-1 / 10);
      uint8_t limit_digit = ((uint8_t)-1 % 10);
      uint8_t digit       = sv.data[i] - '0';

      // Check for overflow.
      if (tmp > limit || (tmp == limit && digit > limit_digit))
        return 0; // Overflow would occur if we added this digit.

      tmp = tmp * 10 + digit;
    }
  } else { // No overflow possible
    for (size_t i = 0; i < sv.length; ++i) {
      tmp = tmp * 10 + (sv_at(sv, i) - '0');
    }
  }

  *value = tmp;

  return 1;
}

static float sv_pow(float base, float exponent) {
  float result = 1.0;

  if (exponent >= 0) {
    for (int i = 0; i < exponent; i++) {
      result *= base;
    }
  } else {
    for (int i = 0; i > exponent; i--) {
      result /= base;
    }
  }

  return result;
}

int sv_parse_float(cstring_view sv, float* value) {
  if (sv_is_empty(sv)) return 0;
  int negative = 0;
  float num    = 0.0f;

  if (sv_front(sv) == '-') {
    negative = 1;
    sv       = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
  }

  if (sv_front(sv) == '+') {
    sv = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
  }

  if (sv_front(sv) == '.') {
    sv = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
    goto frac;
  }

  if (sv_is_numeric(sv_front(sv))) {
    sv_index_t pos1 = sv_find_first_of(sv, svl("."), 0);
    if (pos1 == SV_NPOS) {
      sv_index_t pos2 = sv_find_first_of(sv, svl("eE"), 0);
      if (pos2 == SV_NPOS) { // no frac and no expon
        if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;

        for (size_t i = 0; i < sv.length; ++i) {
          num = num * 10 + (sv_at(sv, i) - '0');
        }
        goto end;
      } else { // no frac, but expon
        if (sv_find_first_not_of(sv_substr(sv, 0, pos2), svl("0123456789"), 0) != SV_NPOS) return 0;

        for (size_t i = 0; i < pos2; ++i) {
          num = num * 10 + (sv_at(sv, i) - '0');
        }

        sv = sv_remove_prefix(sv, pos2 + 1); // remove e or E
        goto expon;
      }
    } else { // we found '.'
      if (sv_find_first_not_of(sv_substr(sv, 0, pos1), svl("0123456789"), 0) != SV_NPOS) return 0;

      for (size_t i = 0; i < pos1; ++i) {
        num = num * 10 + (sv_at(sv, i) - '0');
      }

      sv = sv_remove_prefix(sv, pos1 + 1); // remove .
      goto frac;
    }

  } else {
    return 0;
  }

frac : {
  if (sv_is_empty(sv)) goto end;
  float fraction = 0.0f;
  float denom    = 1.0f;
  sv_index_t pos = sv_find_first_of(sv, svl("eE"), 0);
  if (pos == SV_NPOS) { // fraction only
    if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;
    for (size_t i = 0; i < sv.length; ++i) {
      fraction = fraction * 10 + (sv_at(sv, i) - '0');
      denom *= 10.0f;
    }
    num = num + (fraction / denom);

    goto end;
  } else { // fraction and exponent
    if (sv_find_first_not_of(sv_substr(sv, 0, pos), svl("0123456789"), 0) != SV_NPOS) return 0;
    for (size_t i = 0; i < pos; ++i) {
      fraction = fraction * 10 + (sv_at(sv, i) - '0');
      denom *= 10.0f;
    }
    num = num + (fraction / denom);

    sv = sv_remove_prefix(sv, pos + 1); // remove e or E
    goto expon;
  }
}
expon : {
  if (sv_is_empty(sv)) return 0; // exponet must have something after it

  int negative_expon = 0;
  if (sv_front(sv) == '-') {
    negative_expon = 1;
    sv             = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
  } else if (sv_front(sv) == '+') {
    sv = sv_remove_prefix(sv, 1);
    if (sv_is_empty(sv)) return 0;
  }

  if (sv_find_first_not_of(sv, svl("0123456789"), 0) != SV_NPOS) return 0;
  float exponent = 0.0f;
  for (size_t i = 0; i < sv.length; ++i) {
    exponent = exponent * 10 + (sv_at(sv, i) - '0');
  }

  num = num * sv_pow(10, negative_expon ? -exponent : exponent);
}
end:

  *value = negative ? -num : num;

  return 1;
}

char* sv_strdup(cstring_view sv) {
  char* str = (char*)malloc(sv.length + 1);
  if (!str) return NULL;
  memcpy(str, sv.data, sv.length + 1);
  str[sv.length] = '\0';
  return str;
}

int sv_read_file(const char* filename, cstring_view* sv) {
  FILE* fh = fopen(filename, "rb");
  if (fh == NULL) {
    return 0;
  }

  fseek(fh, 0L, SEEK_END);
  sv->length = ftell(fh);

  char* b = (char*)malloc(sv->length);

  if (b == NULL) {
    fclose(fh);
    return 0;
  }

  if (sv->length == 0) {
    fclose(fh);
    sv->data = b;
    return 1;
  }

  rewind(fh);

  if (fread(b, sv->length, 1, fh) != 1) {
    free(b);
    fclose(fh);
    return 0;
  }

  sv->data = b;

  fclose(fh);
  return 1;
}

int sv_read_file_stdin(cstring_view* sv) {
  size_t capacity = 100;
  size_t index    = 0;

  char* b = (char*)malloc(capacity * sizeof(char));
  char* tmp_b;

  if (b == NULL) {
    return 0;
  }
  int c = fgetc(stdin);

  while (1) {
    if (feof(stdin)) break;
    if (c == EOF) break;

    b[index++] = c;

    if (index == capacity) {
      capacity *= 2;
      tmp_b = (char*)realloc(b, capacity * sizeof(char));
      if (tmp_b == NULL) {
        free(b);
        return 0;
      }
      b = tmp_b;
    }
    c = fgetc(stdin);
  }

  if (index == 0) {
    free(b);
    return 0;
  }

  tmp_b = (char*)realloc(b, index * sizeof(char));
  if (tmp_b == NULL) {
    free(b);
    return 0;
  }
  b = tmp_b;

  sv->data   = b;
  sv->length = index;

  return 1;
}

void sv_read_file_free(cstring_view sv) { free((char*)sv.data); }

#endif // #ifdef SV_IMPLEMENTATION

#endif // _SV_H_