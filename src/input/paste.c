// Pasted text to terminal bytes.
#include "input/paste.h"

#include "base/buf.h"

char *paste_prepare(const char *utf8, bool bracketed, size_t *out_len) {
  Buf out = {0};
  if (bracketed) buf_append_str(&out, "\x1b[200~");
  for (const char *p = utf8; *p; p++) {
    if (*p == '\r' && p[1] == '\n') continue;  // CRLF: the LF below sends the CR
    if (*p == '\n') {
      buf_append(&out, "\r", 1);
    } else if (!(bracketed && *p == 0x1b)) {
      buf_append(&out, p, 1);
    }
  }
  if (bracketed) buf_append_str(&out, "\x1b[201~");
  *out_len = out.len;
  buf_append(&out, "", 1);
  return out.data;
}
