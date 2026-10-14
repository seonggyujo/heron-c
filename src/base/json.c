// Making, reading, comparing and editing JSON values.
#include "base/json.h"

#include <stdlib.h>
#include <string.h>

#include "base/mem.h"

Json *json_new(JsonType type) {
  Json *v = xcalloc(1, sizeof *v);
  v->type = type;
  return v;
}

Json *json_new_bool(bool value) {
  Json *v = json_new(JSON_BOOL);
  v->boolean = value;
  return v;
}

Json *json_new_number(double value) {
  Json *v = json_new(JSON_NUMBER);
  v->number = value;
  return v;
}

Json *json_new_string(const char *text) {
  Json *v = json_new(JSON_STRING);
  v->len = strlen(text);
  v->text = xmalloc(v->len + 1);
  memcpy(v->text, text, v->len + 1);
  return v;
}

void json_free(Json *v) {
  if (!v) return;
  for (int i = 0; i < v->count; i++) {
    json_free(v->items[i]);
    if (v->keys) free(v->keys[i]);
  }
  free(v->items);
  free(v->keys);
  free(v->text);
  free(v);
}

bool json_is(const Json *v, JsonType type) { return v && v->type == type; }

static int key_index(const Json *object, const char *key) {
  if (!json_is(object, JSON_OBJECT)) return -1;
  for (int i = 0; i < object->count; i++) {
    if (strcmp(object->keys[i], key) == 0) return i;
  }
  return -1;
}

Json *json_get(const Json *object, const char *key) {
  int i = key_index(object, key);
  return i < 0 ? NULL : object->items[i];
}

const char *json_string(const Json *v) { return json_is(v, JSON_STRING) ? v->text : NULL; }

const char *json_text(const Json *v) {
  const char *s = json_string(v);
  return s && s[0] ? s : NULL;
}

bool json_number(const Json *v, double *out) {
  if (!json_is(v, JSON_NUMBER)) return false;
  *out = v->number;
  return true;
}

bool json_true(const Json *v) { return json_is(v, JSON_BOOL) && v->boolean; }

bool json_equal(const Json *a, const Json *b) {
  if (!a || !b || a->type != b->type) return a == b;
  switch (a->type) {
    case JSON_BOOL: return a->boolean == b->boolean;
    case JSON_NUMBER: return a->number == b->number;
    case JSON_STRING: return a->len == b->len && memcmp(a->text, b->text, a->len) == 0;
    case JSON_ARRAY:
      if (a->count != b->count) return false;
      for (int i = 0; i < a->count; i++) {
        if (!json_equal(a->items[i], b->items[i])) return false;
      }
      return true;
    case JSON_OBJECT:
      if (a->count != b->count) return false;
      for (int i = 0; i < a->count; i++) {
        if (!json_equal(a->items[i], json_get(b, a->keys[i]))) return false;
      }
      return true;
    default: return true;
  }
}

Json *json_copy(const Json *v) {
  if (!v) return NULL;
  Json *c = json_new(v->type);
  c->boolean = v->boolean;
  c->number = v->number;
  c->len = v->len;
  if (v->text) {
    size_t n = v->type == JSON_STRING ? v->len : strlen(v->text);
    c->text = xmalloc(n + 1);
    memcpy(c->text, v->text, n + 1);
  }
  for (int i = 0; i < v->count; i++) {
    if (v->type == JSON_OBJECT) json_set(c, v->keys[i], json_copy(v->items[i]));
    else json_push(c, json_copy(v->items[i]));
  }
  return c;
}

static void grow(Json *v) {
  if (v->count < v->cap) return;
  v->cap = v->cap ? v->cap * 2 : 4;
  v->items = xrealloc(v->items, sizeof(Json *) * (size_t)v->cap);
  if (v->type == JSON_OBJECT) v->keys = xrealloc(v->keys, sizeof(char *) * (size_t)v->cap);
}

void json_set(Json *object, const char *key, Json *value) {
  int i = key_index(object, key);
  if (i >= 0) {
    json_free(object->items[i]);
    object->items[i] = value;
    return;
  }
  grow(object);
  object->keys[object->count] = xmalloc(strlen(key) + 1);
  strcpy(object->keys[object->count], key);
  object->items[object->count++] = value;
}

void json_set_copy(Json *object, const char *key, const Json *v) {
  json_set(object, key, v ? json_copy(v) : json_new(JSON_NULL));
}

void json_push(Json *array, Json *value) {
  grow(array);
  array->items[array->count++] = value;
}

void json_remove_at(Json *container, int index) {
  if (index < 0 || index >= container->count) return;
  json_free(container->items[index]);
  if (container->keys) free(container->keys[index]);
  int rest = container->count - index - 1;
  memmove(&container->items[index], &container->items[index + 1], sizeof(Json *) * (size_t)rest);
  if (container->keys) memmove(&container->keys[index], &container->keys[index + 1], sizeof(char *) * (size_t)rest);
  container->count--;
}

bool json_remove(Json *object, const char *key) {
  int i = key_index(object, key);
  if (i < 0) return false;
  json_remove_at(object, i);
  return true;
}
