// Strict JSON parsing (RFC 8259): one value, nesting up to 128 deep.
// Numbers and strings are read in base/json_scalars.c.
#include "base/json_reader.h"

#define MAX_DEPTH 128

static void skip_space(JsonReader *r) {
  while (r->at < r->end && (*r->at == ' ' || *r->at == '\t' || *r->at == '\n' || *r->at == '\r')) r->at++;
}

static Json *parse_value(JsonReader *r);

static Json *parse_container(JsonReader *r, bool object) {
  if (++r->depth > MAX_DEPTH) return NULL;
  Json *v = json_new(object ? JSON_OBJECT : JSON_ARRAY);
  char close = object ? '}' : ']';
  skip_space(r);
  if (r->at < r->end && *r->at == close) {
    r->at++;
    r->depth--;
    return v;
  }
  for (;;) {
    Buf key = {0};
    skip_space(r);
    if (object && !(json_reader_take(r, "\"") && json_read_string(r, &key) &&
                    (skip_space(r), json_reader_take(r, ":")))) {
      buf_free(&key);
      break;
    }
    Json *item = parse_value(r);
    if (!item) {
      buf_free(&key);
      break;
    }
    if (object) json_set(v, key.data, item);
    else json_push(v, item);
    buf_free(&key);
    skip_space(r);
    if (json_reader_take(r, ",")) continue;
    if (json_reader_take(r, object ? "}" : "]")) {
      r->depth--;
      return v;
    }
    break;
  }
  json_free(v);
  return NULL;
}

static Json *parse_value(JsonReader *r) {
  skip_space(r);
  if (r->at >= r->end) return NULL;
  switch (*r->at) {
    case '{': r->at++; return parse_container(r, true);
    case '[': r->at++; return parse_container(r, false);
    case '"': {
      r->at++;
      Buf s = {0};
      if (!json_read_string(r, &s)) {
        buf_free(&s);
        return NULL;
      }
      Json *v = json_new(JSON_STRING);
      v->text = s.data;
      v->len = s.len - 1;
      return v;
    }
    case 't': return json_reader_take(r, "true") ? json_new_bool(true) : NULL;
    case 'f': return json_reader_take(r, "false") ? json_new_bool(false) : NULL;
    case 'n': return json_reader_take(r, "null") ? json_new(JSON_NULL) : NULL;
    default: return json_read_number(r);
  }
}

Json *json_parse(const char *text, size_t len) {
  JsonReader r = {text, text + len, 0};
  Json *v = parse_value(&r);
  skip_space(&r);
  if (v && r.at != r.end) {
    json_free(v);
    return NULL;
  }
  return v;
}
