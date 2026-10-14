// Tests for parsing, editing and writing JSON.
#include <stdlib.h>
#include <string.h>

#include "base/json.h"
#include "check.h"

static Json *parse(const char *text) { return json_parse(text, strlen(text)); }

// Written back, pretty or compact; the caller frees.
static char *written(const Json *v, bool pretty) {
  Buf out = {0};
  json_write(v, &out, pretty);
  buf_append(&out, "", 1);
  return out.data;
}

void test_json_round_trip(void) {
  const char *file =
      "{\n  \"model\": \"opus\",\n  \"hooks\": {\n    \"Stop\": [\n      {\n        \"hooks\": []\n      }\n"
      "    ]\n  },\n  \"n\": 1.50,\n  \"e\": -2E+3,\n  \"empty\": {},\n  \"ok\": true,\n  \"none\": null\n}";
  Json *v = parse(file);
  char *back = written(v, true);
  CHECK_STR(back, file);  // keys in order, numbers as written
  free(back);
  double n;
  CHECK(json_number(json_get(v, "e"), &n) && n == -2000);
  json_free(v);
  v = parse("[\"a\",[\"b\"],{\"c\":[\"d\",1]}]");  // strings in arrays are values, not keys
  back = written(v, false);
  CHECK_STR(back, "[\"a\",[\"b\"],{\"c\":[\"d\",1]}]");
  free(back);
  json_free(v);
  v = parse("{\"a\":1,\"b\":2,\"a\":3}");  // a duplicate keeps its first place
  back = written(v, false);
  CHECK_STR(back, "{\"a\":3,\"b\":2}");
  free(back);
  json_free(v);
}

void test_json_strings(void) {
  Json *v = parse("\"\\u00e9\\ud83d\\ude00 \\\"q\\\" \\/ \\n\\u0001\"");
  CHECK_STR(json_string(v), "\xc3\xa9\xf0\x9f\x98\x80 \"q\" / \n\x01");
  char *back = written(v, false);
  CHECK_STR(back, "\"\xc3\xa9\xf0\x9f\x98\x80 \\\"q\\\" / \\n\\u0001\"");
  free(back);
  json_free(v);
  const char *bad[] = {"{\"a\":1,}", "[1 2]", "\"\\ud800\"", "\"a\nb\"", "01", "1.", "{} x", "", "nul"};
  for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) CHECK(parse(bad[i]) == NULL);
}

void test_json_edit(void) {
  Json *v = parse("{\"keep\":[1],\"hooks\":{}}");
  json_set(json_get(v, "hooks"), "Stop", json_new(JSON_ARRAY));
  json_push(json_get(json_get(v, "hooks"), "Stop"), json_new_string("x"));
  json_set(v, "n", json_new_number(10));
  json_set(v, "keep", json_new_bool(false));
  char *back = written(v, false);
  CHECK_STR(back, "{\"keep\":false,\"hooks\":{\"Stop\":[\"x\"]},\"n\":10}");
  free(back);
  Json *copy = json_copy(v);
  CHECK(json_equal(copy, v));
  CHECK(json_remove(copy, "hooks") && !json_equal(copy, v));
  back = written(copy, true);
  CHECK_STR(back, "{\n  \"keep\": false,\n  \"n\": 10\n}");
  free(back);
  json_free(copy);
  json_free(v);
}
