// The pane header, left to right: status dot, shell name, exit badge,
// current folder, program title, then the uptime and the close button.
// One layout serves drawing and the folder and title tooltips.
#include <stdio.h>
#include <string.h>

#include "app/strings.h"
#include "app/theme.h"
#include "pane/pane_paint.h"

// Where each item starts and how wide it is drawn (0 when it is not shown).
typedef struct {
  float dot_x, shell_x, badge_x, badge_w;
  float cwd_x, cwd_w, sep_x, title_x, title_w, uptime_x;
  char badge[48], uptime[32];
} HeaderLayout;

// The shell ended before the pane was closed. One ended by the close
// itself keeps its running look while the pane fades out.
static bool shows_exit(const Pane *p) { return p->exited && (!p->closed_at || p->ended_at < p->closed_at); }

static float text_width(UiFont *f, const char *text, float max_w) {
  float w = ui_text_width(f, text);
  return w < max_w ? w : max_w > 0 ? max_w : 0;
}

static void layout(const Pane *p, const PaneStyle *s, HeaderLayout *l) {
  UiFont *f = s->header_font;
  float k = s->scale, gap = 8 * k;
  memset(l, 0, sizeof *l);
  l->dot_x = p->frame.x + 10 * k;
  l->shell_x = l->dot_x + 8 * k + gap;
  float left = l->shell_x + ui_text_width(f, shell_name(p->shell)) + gap;
  if (shows_exit(p) && (p->exit_code != 0 || !p->pty)) {
    if (p->pty) format_exit_code(l->badge, sizeof l->badge, p->exit_code);
    else snprintf(l->badge, sizeof l->badge, "%s", strings()->failed_to_start);
    l->badge_x = left;
    l->badge_w = ui_text_width(f, l->badge) + 14 * k;
    left += l->badge_w + gap;
  }
  ULONGLONG end = shows_exit(p) ? p->ended_at : p->closed_at ? p->closed_at : s->now;
  format_uptime(l->uptime, sizeof l->uptime, end - p->started_at);
  l->uptime_x = pane_close_box(p, s->m).x - 6 * k - ui_text_width(f, l->uptime);
  float limit = l->uptime_x - gap;
  const char *cwd = term_cwd(p->term), *title = term_title(p->term);
  if (cwd[0] && left < limit) {
    l->cwd_x = left;
    l->cwd_w = text_width(f, cwd, (limit - left) * 0.45f);
    left += l->cwd_w;
  }
  if (title[0] && left < limit) {
    if (cwd[0]) {
      l->sep_x = left + gap + 1 * k;
      left = l->sep_x + ui_text_width(f, "\xc2\xb7") + 7 * k;
    }
    l->title_x = left;
    l->title_w = text_width(f, title, limit - left);
  }
}

bool pane_header_tip(const Pane *p, const PaneStyle *s, int x, int y, Box *box, const char **text) {
  if (y < p->frame.y || y >= p->frame.y + s->m->header_h) return false;
  HeaderLayout l;
  layout(p, s, &l);
  Box cwd = {(int)l.cwd_x, p->frame.y, (int)l.cwd_w, s->m->header_h};
  Box title = {(int)l.title_x, p->frame.y, (int)l.title_w, s->m->header_h};
  if (l.cwd_w > 0 && box_contains(cwd, x, y)) {
    *box = cwd;
    *text = term_cwd(p->term);
    return true;
  }
  if (l.title_w > 0 && box_contains(title, x, y)) {
    *box = title;
    *text = term_title(p->term);
    return true;
  }
  return false;
}

static void draw_text(Canvas *c, UiFont *f, const char *text, float x, float w, float y, float h, uint32_t color) {
  if (w > 0) ui_text_draw(c, f, text, x, y, w + 1, h, color, ALIGN_LEFT);
}

static void draw_close(Canvas *c, Box b, bool hover, float k) {
  if (hover) canvas_fill_round(c, (float)b.x, (float)b.y, (float)b.w, (float)b.h, 6 * k, THEME_ERROR_BG);
  float cx = b.x + b.w / 2.0f, cy = b.y + b.h / 2.0f, d = 4 * k;
  uint32_t color = hover ? THEME_ERROR_FG : THEME_FG_DIM;
  canvas_line(c, cx - d, cy - d, cx + d, cy + d, 1.3f * k, color);
  canvas_line(c, cx - d, cy + d, cx + d, cy - d, 1.3f * k, color);
}

void pane_header_paint(Canvas *c, const Pane *p, const PaneStyle *s, PaneState st) {
  const PaneMetrics *m = s->m;
  UiFont *f = s->header_font;
  float k = s->scale, dot = 4 * k;
  float x = (float)p->frame.x, y = (float)p->frame.y, w = (float)p->frame.w, h = (float)m->header_h;
  float cy = y + h / 2;
  canvas_fill_round(c, x + 1, y + 1, w - 2, h + m->radius, m->radius - 1.0f, THEME_RAISED);
  canvas_fill_rect(c, x + 1, y + h - 1, w - 2, 1, THEME_BORDER_SOFT);
  HeaderLayout l;
  layout(p, s, &l);

  if (p->pty && !shows_exit(p)) {
    uint32_t alpha = (uint32_t)(255 * st.dot), color = st.dot_color ? st.dot_color : THEME_FG;
    canvas_fill_circle(c, l.dot_x + dot, cy, dot, (alpha << 24) | (color & 0xFFFFFF));
  } else {
    canvas_stroke_circle(c, l.dot_x + dot, cy, dot, 1.5f * k, THEME_FG_MUTED);
  }
  const char *shell = shell_name(p->shell);
  draw_text(c, f, shell, l.shell_x, ui_text_width(f, shell), y, h, st.selected ? THEME_FG : THEME_FG_DIM);
  if (l.badge_w > 0) {
    float bh = 18 * k;
    canvas_fill_round(c, l.badge_x, cy - bh / 2, l.badge_w, bh, bh / 2, THEME_ERROR_BG);
    ui_text_draw(c, f, l.badge, l.badge_x, cy - bh / 2, l.badge_w, bh, THEME_ERROR_FG, ALIGN_CENTER);
  }
  draw_text(c, f, term_cwd(p->term), l.cwd_x, l.cwd_w, y, h, THEME_FG_SOFT);
  if (l.sep_x > 0) draw_text(c, f, "\xc2\xb7", l.sep_x, ui_text_width(f, "\xc2\xb7"), y, h, THEME_FG_DIM);  // ·
  draw_text(c, f, term_title(p->term), l.title_x, l.title_w, y, h, st.selected ? THEME_FG_SOFT : THEME_FG_DIM);
  draw_text(c, f, l.uptime, l.uptime_x, ui_text_width(f, l.uptime), y, h, THEME_FG_DIM);
  draw_close(c, pane_close_box(p, m), st.close_hover, k);
}
