// JSON values: parse, read, edit and write. Objects keep their keys in the
// order they came (a duplicate key replaces the value in its first place),
// so a user's file written back keeps its shape. A number keeps the text
// it was written as and is written back the same. Strings are UTF-8 with a
// length (they may hold \u0000). Written JSON is compact, or indented by
// two spaces like Claude Code and serde_json write settings.json.
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base/buf.h"

typedef enum { JSON_NULL, JSON_BOOL, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT } JsonType;

typedef struct Json {
  JsonType type;
  bool boolean;
  double number;
  char *text;  // STRING: the value; NUMBER: as written, NULL for one made here
  size_t len;  // STRING: bytes in text
  int count, cap;
  struct Json **items;  // ARRAY: the items; OBJECT: the values
  char **keys;          // OBJECT: the keys
} Json;

// One JSON value with only whitespace around it; NULL for anything else.
Json *json_parse(const char *text, size_t len);
void json_free(Json *v);

// Reading. Every function takes NULL and then finds nothing.
bool json_is(const Json *v, JsonType type);
Json *json_get(const Json *object, const char *key);
// The string, NULL unless v is one.
const char *json_string(const Json *v);
// The string when it is not empty, else NULL (Heron's `text()`).
const char *json_text(const Json *v);
bool json_number(const Json *v, double *out);
bool json_true(const Json *v);
// Equal values (numbers by value, objects in any key order).
bool json_equal(const Json *a, const Json *b);

// Building and editing. Functions that take a Json* value own it after.
Json *json_new(JsonType type);
Json *json_new_bool(bool value);
Json *json_new_number(double value);
Json *json_new_string(const char *text);
Json *json_copy(const Json *v);
// Replaces the value of key in its place, or adds it at the end.
void json_set(Json *object, const char *key, Json *value);
// Adds a copy of v (or null when v is NULL).
void json_set_copy(Json *object, const char *key, const Json *v);
bool json_remove(Json *object, const char *key);
void json_push(Json *array, Json *value);
void json_remove_at(Json *container, int index);

// json_write.c
void json_write(const Json *v, Buf *out, bool pretty);
