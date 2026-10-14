// Runs every unit test and exits non-zero if any CHECK failed.
#include <stdio.h>

#include "check.h"

int g_failures;

#define TESTS(X)                 \
  X(test_vt_print_and_wrap)      \
  X(test_vt_wide_chars)          \
  X(test_vt_split_utf8)          \
  X(test_vt_history)             \
  X(test_vt_scroll_region)       \
  X(test_vt_alt_screen)          \
  X(test_vt_resize)              \
  X(test_vt_selection)           \
  X(test_vt_cursor_and_erase)    \
  X(test_vt_colors)              \
  X(test_vt_replies)             \
  X(test_vt_modes)               \
  X(test_vt_osc)                 \
  X(test_vt_charset_and_tabs)    \
  X(test_keys)                   \
  X(test_mouse_reports)          \
  X(test_paste)                  \
  X(test_url_scan)               \
  X(test_term_links)             \
  X(test_split_tree)             \
  X(test_split_rects)            \
  X(test_drop_zone)              \
  X(test_auto_grid)              \
  X(test_arrangement)            \
  X(test_divider_drag)           \
  X(test_easing)                 \
  X(test_json_round_trip)        \
  X(test_json_strings)           \
  X(test_json_edit)              \
  X(test_hook_config)            \
  X(test_hook_config_leaves_others) \
  X(test_hook_states)            \
  X(test_hook_models_and_subagents) \
  X(test_answers)                \
  X(test_model_check)            \
  X(test_transcript)             \
  X(test_agent_titles)           \
  X(test_agent_hooks)            \
  X(test_agent_usage)            \
  X(test_session_summary)        \
  X(test_numstat)                \
  X(test_paths)                  \
  X(test_session_groups)         \
  X(test_time_texts)             \
  X(test_tree_shape)             \
  X(test_saved_work)             \
  X(test_buddy_sprites)          \
  X(test_buddy_ground)           \
  X(test_buddy_crew)             \
  X(test_buddy_flock)

#define DECLARE(name) void name(void);
TESTS(DECLARE)

#define RUN(name)                      \
  do {                                 \
    int before = g_failures;           \
    name();                            \
    if (g_failures != before) fprintf(stderr, "  in %s\n", #name); \
    count++;                           \
  } while (0);

int main(void) {
  int count = 0;
  TESTS(RUN)
  printf("%d tests, %d failed checks\n", count, g_failures);
  return g_failures ? 1 : 0;
}
