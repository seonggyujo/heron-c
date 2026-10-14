// Reading and writing the saved work.
#include "app/workspace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/file.h"
#include "base/mem.h"
#include "base/utf.h"

static void copy_text(char *out, size_t cap, const Json *v) {
  const char *s = json_text(v);
  snprintf(out, cap, "%s", s ? s : "");
}

bool workspace_parse(const char *text, SavedWorkspace *out) {
  memset(out, 0, sizeof *out);
  Json *saved = text ? json_parse(text, strlen(text)) : NULL;
  const Json *panes = json_get(saved, "panes");
  if (!json_is(panes, JSON_ARRAY)) {
    json_free(saved);
    return false;
  }
  out->panes = xcalloc((size_t)panes->count + 1, sizeof *out->panes);
  for (int i = 0; i < panes->count; i++) {
    SavedPane *p = &out->panes[out->count++];
    copy_text(p->shell, sizeof p->shell, json_get(panes->items[i], "shell"));
    copy_text(p->cwd, sizeof p->cwd, json_get(panes->items[i], "cwd"));
    copy_text(p->session, sizeof p->session, json_get(panes->items[i], "session"));
  }
  const Json *layout = json_get(saved, "layout");
  out->layout = layout && layout->type != JSON_NULL ? json_copy(layout) : NULL;
  json_free(saved);
  return true;
}

void workspace_free(SavedWorkspace *w) {
  free(w->panes);
  json_free(w->layout);
  memset(w, 0, sizeof *w);
}

static Json *text_or_null(const char *s) { return s[0] ? json_new_string(s) : json_new(JSON_NULL); }

char *workspace_text(const SavedPane *panes, int count, const Json *layout) {
  Json *root = json_new(JSON_OBJECT), *list = json_new(JSON_ARRAY);
  for (int i = 0; i < count; i++) {
    Json *p = json_new(JSON_OBJECT);
    json_set(p, "shell", json_new_string(panes[i].shell));
    json_set(p, "cwd", text_or_null(panes[i].cwd));
    json_set(p, "session", text_or_null(panes[i].session));
    json_push(list, p);
  }
  json_set(root, "panes", list);
  json_set_copy(root, "layout", layout);
  Buf out = {0};
  json_write(root, &out, false);
  buf_append(&out, "", 1);
  json_free(root);
  return out.data;
}

static bool path(char *out, size_t cap) {
  wchar_t base[MAX_PATH];
  DWORD n = GetEnvironmentVariableW(L"APPDATA", base, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return false;
  char *utf8 = wide_to_utf8(base, -1, NULL);
  snprintf(out, cap, "%s\\heron-c\\workspace.json", utf8);
  free(utf8);
  return true;
}

char *workspace_read(void) {
  char file[MAX_PATH * 3];
  return path(file, sizeof file) ? file_read(file, NULL) : NULL;
}

void workspace_write(const char *text) {
  char file[MAX_PATH * 3], dir[MAX_PATH * 3];
  if (!path(file, sizeof file)) return;
  snprintf(dir, sizeof dir, "%.*s", (int)(strrchr(file, '\\') - file), file);
  file_make_dirs(dir);
  file_write_atomic(file, text, strlen(text));
}
