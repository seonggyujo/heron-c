// Dropped paths to pane input.
#include "ui/file_drop_paths.h"

#include <stdlib.h>
#include <string.h>

#include "base/buf.h"
#include "base/log.h"
#include "base/mem.h"
#include "base/utf.h"
#include "pane/pane_input.h"

static bool needs_quotes(const char *path) {
  for (const char *c = path; *c; c++) {
    if ((unsigned char)*c <= ' ' || strchr("&()[]{}^=;!'+,`~", *c)) return true;
  }
  return false;
}

void drop_paths_paste(App *a, Pane *pane, HDROP drop) {
  UINT count = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
  Buf text = {0};
  for (UINT i = 0; i < count; i++) {
    UINT len = DragQueryFileW(drop, i, NULL, 0);
    wchar_t *w = xmalloc((len + 1) * sizeof(wchar_t));
    DragQueryFileW(drop, i, w, len + 1);
    char *path = wide_to_utf8(w, -1, NULL);
    bool quote = needs_quotes(path);
    if (quote) buf_append_str(&text, "\"");
    buf_append_str(&text, path);
    buf_append_str(&text, quote ? "\" " : " ");
    free(path);
    free(w);
  }
  if (count) {
    buf_append(&text, "", 1);
    pane_paste(pane, text.data);
    log_info("file-drop", "dropped %u path(s) on pane %d", count, pane->id);
  }
  buf_free(&text);
  app_redraw(a);
}
