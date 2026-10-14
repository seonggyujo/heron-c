// Clipboard text through CF_UNICODETEXT.
#include "ui/clipboard.h"

#include <stdlib.h>
#include <string.h>

#include "base/log.h"
#include "base/utf.h"

bool clipboard_set_text(HWND owner, const char *utf8) {
  wchar_t *text = utf8_to_wide(utf8, -1);
  size_t bytes = (wcslen(text) + 1) * sizeof(wchar_t);
  HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
  bool ok = false;
  if (mem) {
    memcpy(GlobalLock(mem), text, bytes);
    GlobalUnlock(mem);
    if (OpenClipboard(owner)) {
      EmptyClipboard();
      ok = SetClipboardData(CF_UNICODETEXT, mem) != NULL;
      CloseClipboard();
    }
    if (!ok) GlobalFree(mem);  // the clipboard owns it only on success
  }
  free(text);
  if (!ok) log_warn("clipboard", "copy failed: error %lu", GetLastError());
  return ok;
}

char *clipboard_get_text(HWND owner) {
  if (!OpenClipboard(owner)) return NULL;
  char *text = NULL;
  HANDLE data = GetClipboardData(CF_UNICODETEXT);
  const wchar_t *w = data ? GlobalLock(data) : NULL;
  if (w) {
    text = wide_to_utf8(w, -1, NULL);
    GlobalUnlock(data);
  }
  CloseClipboard();
  return text;
}
