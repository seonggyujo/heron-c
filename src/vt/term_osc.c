// OSC sequences: window title (0, 2), current folder (7, and 9;9 as Windows
// Terminal uses it) and the default color queries (10, 11).
#include <ctype.h>
#include <string.h>

#include "vt/palette.h"
#include "vt/term_internal.h"

// Copies at most cap - 1 bytes without cutting a UTF-8 character in half.
static void copy_utf8(char *dst, size_t cap, const char *src, size_t len) {
  if (len >= cap) {
    len = cap - 1;
    while (len > 0 && ((unsigned char)src[len] & 0xC0) == 0x80) len--;
  }
  memcpy(dst, src, len);
  dst[len] = 0;
}

static int hex_digit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// Turns "/C:/x" (OSC 7) and "/c/x" (Git Bash) into "C:\x". Windows paths
// pass through. url means %XX escapes are decoded first.
static void set_cwd(Term *t, const char *path, size_t len, bool url) {
  char tmp[1024];
  size_t n = 0;
  for (size_t i = 0; i < len && n < sizeof tmp - 2; i++) {
    int hi = url && path[i] == '%' && i + 2 < len ? hex_digit(path[i + 1]) : -1;
    int lo = hi >= 0 ? hex_digit(path[i + 2]) : -1;
    if (lo >= 0) {
      tmp[n++] = (char)(hi * 16 + lo);
      i += 2;
    } else {
      tmp[n++] = path[i];
    }
  }
  tmp[n] = 0;
  char *p = tmp;
  if (p[0] == '/' && isalpha((unsigned char)p[1]) && p[2] == ':') {
    p++;
  } else if (p[0] == '/' && isalpha((unsigned char)p[1]) && (p[2] == '/' || p[2] == 0)) {
    p[0] = p[1];
    p[1] = ':';
    if (p[2] == 0) strcpy(p + 2, "\\");
  }
  if (isalpha((unsigned char)p[0]) && p[1] == ':') p[0] = (char)toupper((unsigned char)p[0]);
  for (char *c = p; *c; c++) {
    if (*c == '/') *c = '\\';
  }
  if (!p[0]) return;
  // Shells report their folder before every prompt (pty/shell_launch.c).
  t->events |= TERM_EVENT_PROMPT;
  if (strcmp(t->cwd, p) == 0) return;
  copy_utf8(t->cwd, sizeof t->cwd, p, strlen(p));
  t->events |= TERM_EVENT_CWD;
}

static void reply_color(Term *t, int which) {
  uint32_t rgb = which == 10 ? PALETTE_FG : PALETTE_BG;
  unsigned r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
  buf_printf(&t->replies, "\x1b]%d;rgb:%02x%02x/%02x%02x/%02x%02x\x1b\\", which, r, r, g, g, b, b);
}

void term_osc(void *user, const char *data, size_t len) {
  Term *t = user;
  int num = 0;
  size_t i = 0;
  while (i < len && i < 6 && data[i] >= '0' && data[i] <= '9') num = num * 10 + (data[i++] - '0');
  if (i == 0 || (i < len && data[i] != ';')) return;
  const char *text = i < len ? data + i + 1 : data + len;
  size_t text_len = len - (size_t)(text - data);
  switch (num) {
    case 0:
    case 2:
      copy_utf8(t->title, sizeof t->title, text, text_len);
      t->events |= TERM_EVENT_TITLE;
      break;
    case 7:
      if (text_len > 7 && strncmp(text, "file://", 7) == 0) {
        const char *slash = memchr(text + 7, '/', text_len - 7);
        if (slash) set_cwd(t, slash, text_len - (size_t)(slash - text), true);
      }
      break;
    case 8:
      term_hyperlink(t, text, text_len);
      break;
    case 9:
      if (text_len > 2 && text[0] == '9' && text[1] == ';') set_cwd(t, text + 2, text_len - 2, false);
      break;
    case 10:
    case 11:
      if (text_len == 1 && text[0] == '?') reply_color(t, num);
      break;
  }
}
