// SGR (CSI ... m): colors and attributes for the characters that follow.
#include "vt/term_internal.h"

// Number of sub-parameters (':' form) right after index i.
static int sub_count(const VtParams *p, int i) {
  int n = 0;
  while (i + 1 + n < p->n && p->colon[i + 1 + n]) n++;
  return n;
}

static int byte_at(const VtParams *p, int i) {
  int v = vt_param(p, i, 0);
  return v > 255 ? 255 : v;
}

// Reads the color after a 38 or 48 at index i. Handles 38;5;n and 38;2;r;g;b
// and the colon forms 38:5:n, 38:2:r:g:b and 38:2:space:r:g:b. Returns how
// many following values it used.
static int extended_color(const VtParams *p, int i, uint32_t *out) {
  int subs = sub_count(p, i);
  if (subs > 0) {
    int kind = vt_param(p, i + 1, 0);
    if (kind == 5 && subs >= 2) *out = COLOR_INDEX(byte_at(p, i + 2));
    if (kind == 2 && subs >= 5) {
      *out = COLOR_RGB(byte_at(p, i + 3), byte_at(p, i + 4), byte_at(p, i + 5));
    } else if (kind == 2 && subs == 4) {
      *out = COLOR_RGB(byte_at(p, i + 2), byte_at(p, i + 3), byte_at(p, i + 4));
    }
    return subs;
  }
  int kind = vt_param(p, i + 1, 0);
  if (kind == 5 && i + 2 < p->n) {
    *out = COLOR_INDEX(byte_at(p, i + 2));
    return 2;
  }
  if (kind == 2 && i + 4 < p->n) {
    *out = COLOR_RGB(byte_at(p, i + 2), byte_at(p, i + 3), byte_at(p, i + 4));
    return 4;
  }
  return p->n - i - 1;  // malformed: skip the rest
}

static void set_attr(Cell *pen, uint16_t attr, bool on) {
  pen->attr = (uint16_t)(on ? (pen->attr | attr) : (pen->attr & ~attr));
}

void term_sgr(Term *t, const VtParams *p) {
  Cell *pen = &t->pen;
  if (p->n == 0) {
    pen->fg = pen->bg = COLOR_DEFAULT;
    pen->attr = 0;
    return;
  }
  for (int i = 0; i < p->n; i++) {
    if (p->colon[i]) continue;  // sub-parameter, read with its parent
    int v = vt_param(p, i, 0);
    uint32_t color = COLOR_DEFAULT;
    if (v >= 30 && v <= 37) {
      pen->fg = COLOR_INDEX(v - 30);
    } else if (v >= 40 && v <= 47) {
      pen->bg = COLOR_INDEX(v - 40);
    } else if (v >= 90 && v <= 97) {
      pen->fg = COLOR_INDEX(v - 90 + 8);
    } else if (v >= 100 && v <= 107) {
      pen->bg = COLOR_INDEX(v - 100 + 8);
    } else {
      switch (v) {
        case 0: pen->fg = pen->bg = COLOR_DEFAULT; pen->attr = 0; break;
        case 1: set_attr(pen, ATTR_BOLD, true); break;
        case 2: set_attr(pen, ATTR_DIM, true); break;
        case 3: set_attr(pen, ATTR_ITALIC, true); break;
        case 4:  // 4:0 turns underline off; 4:1 to 4:5 are underline styles
          set_attr(pen, ATTR_UNDERLINE, !(sub_count(p, i) > 0 && vt_param(p, i + 1, 1) == 0));
          break;
        case 5:
        case 6: set_attr(pen, ATTR_BLINK, true); break;
        case 7: set_attr(pen, ATTR_INVERSE, true); break;
        case 8: set_attr(pen, ATTR_HIDDEN, true); break;
        case 9: set_attr(pen, ATTR_STRIKE, true); break;
        case 21: set_attr(pen, ATTR_UNDERLINE, true); break;
        case 22: set_attr(pen, ATTR_BOLD | ATTR_DIM, false); break;
        case 23: set_attr(pen, ATTR_ITALIC, false); break;
        case 24: set_attr(pen, ATTR_UNDERLINE, false); break;
        case 25: set_attr(pen, ATTR_BLINK, false); break;
        case 27: set_attr(pen, ATTR_INVERSE, false); break;
        case 28: set_attr(pen, ATTR_HIDDEN, false); break;
        case 29: set_attr(pen, ATTR_STRIKE, false); break;
        case 38: i += extended_color(p, i, &color); pen->fg = color; break;
        case 39: pen->fg = COLOR_DEFAULT; break;
        case 48: i += extended_color(p, i, &color); pen->bg = color; break;
        case 49: pen->bg = COLOR_DEFAULT; break;
        case 58: i += extended_color(p, i, &color); break;  // underline color: not drawn
      }
    }
  }
}
