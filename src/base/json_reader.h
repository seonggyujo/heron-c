// The JSON parser's cursor and the readers of single values it shares
// between base/json_parse.c (arrays, objects) and base/json_scalars.c
// (numbers, strings). Only base/json_*.c include this.
#pragma once

#include <stdbool.h>

#include "base/buf.h"
#include "base/json.h"

typedef struct {
  const char *at, *end;
  int depth;
} JsonReader;

// Steps over word when the text continues with it.
bool json_reader_take(JsonReader *r, const char *word);
// The number at r, NULL when there is none.
Json *json_read_number(JsonReader *r);
// After the opening quote: the string's bytes into out, NUL-terminated.
bool json_read_string(JsonReader *r, Buf *out);
