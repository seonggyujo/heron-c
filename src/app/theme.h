// Colors of the app frame (title bar, pane borders and headers, sidebar),
// 0xAARRGGBB, in two themes: Heron's sage green and a neutral black. The
// terminal inside keeps its own default colors (vt/palette.h); the theme
// never reaches it. THEME_* read the current theme, so callers need not
// know which one is on.
#pragma once

#include <stdint.h>

typedef enum { THEME_GREEN, THEME_BLACK } ThemeKind;

typedef struct {
  uint32_t bg_app, bg_pane, bar, bar_bottom, raised, control, hover, active, inset;
  uint32_t border, border_soft, border_strong;
  uint32_t fg, fg_soft, fg_dim, fg_muted;
  uint32_t glow, flash;  // focus glow and output flash, with alpha
  uint32_t badge_bg, badge_fg, light_off, light_off_ring;
} ThemeColors;

const ThemeColors *theme_colors(void);
ThemeKind theme_kind(void);
void theme_set(ThemeKind kind);

#define THEME_BG_APP (theme_colors()->bg_app)
#define THEME_BG_PANE (theme_colors()->bg_pane)
#define THEME_BAR (theme_colors()->bar)
#define THEME_BAR_BOTTOM (theme_colors()->bar_bottom)  // sidebar and usage bar
#define THEME_RAISED (theme_colors()->raised)
#define THEME_CONTROL (theme_colors()->control)
#define THEME_HOVER (theme_colors()->hover)
#define THEME_ACTIVE (theme_colors()->active)
#define THEME_INSET (theme_colors()->inset)
#define THEME_BORDER (theme_colors()->border)
#define THEME_BORDER_SOFT (theme_colors()->border_soft)
#define THEME_BORDER_STRONG (theme_colors()->border_strong)
#define THEME_FG (theme_colors()->fg)
#define THEME_FG_SOFT (theme_colors()->fg_soft)
#define THEME_FG_DIM (theme_colors()->fg_dim)
#define THEME_FG_MUTED (theme_colors()->fg_muted)
#define THEME_GLOW (theme_colors()->glow)
#define THEME_FLASH (theme_colors()->flash)
#define THEME_BADGE_BG (theme_colors()->badge_bg)
#define THEME_BADGE_FG (theme_colors()->badge_fg)
#define THEME_LIGHT_OFF (theme_colors()->light_off)
#define THEME_LIGHT_OFF_RING (theme_colors()->light_off_ring)

// The same in both themes.
#define THEME_ERROR_BG 0xFF3A1616u
#define THEME_ERROR_FG 0xFFFF8F8Fu
#define THEME_ERROR_BORDER 0xFF6B2A2Au
#define THEME_LIGHT_CLOSE 0xFFFF5F57u
#define THEME_LIGHT_MIN 0xFFFEBC2Eu
#define THEME_LIGHT_MAX 0xFF28C840u
#define THEME_LIGHT_GLYPH 0x99000000u
#define THEME_LIGHT_RING 0x59000000u

// Coding agent states: working, needs you, done, wrong model or limit
// nearly used.
#define THEME_AGENT_WORK 0xFF8FD8A8u
#define THEME_AGENT_NEED 0xFFE6B450u
#define THEME_AGENT_DONE 0xFF7FB8E6u
#define THEME_AGENT_BAD 0xFFEF6F6Cu
