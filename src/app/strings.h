// Every piece of UI text, in English or Korean (the same keys as Heron's
// i18n/en.ts and ko.ts). Without a saved choice the language follows the
// Windows display language. Shell names and logs are not translated.
#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum { LANG_EN, LANG_KO } Lang;

typedef struct {
  const char *minimize, *maximize, *close, *new_terminal, *choose_shell, *tidy;
  const char *empty_caption, *close_terminal, *failed_to_start;
  const char *settings, *theme, *theme_green, *theme_black, *language, *font_size, *font_smaller,
      *font_larger, *animations, *buddies, *buddies_off, *buddies_hint, *close_on_exit, *close_on_exit_hint,
      *restore_workspace, *restore_workspace_hint, *agent_hooks, *agent_hooks_hint, *reset, *reset_hint, *sure,
      *done;
  const char *agents, *collapse, *expand, *agent_idle, *agent_working, *agent_permission,
      *agent_question, *agent_waiting, *agent_done, *agent_plugin_hint, *hooks_offer, *hooks_turn_on,
      *hooks_turning_on, *hooks_no_claude, *hooks_bad_settings, *hooks_failed, *hide, *new_agent,
      *choose_agent_folder, *agent_folders, *pick_folder, *no_agents, *recent_sessions, *recent_hint,
      *resume, *just_now, *delete_hint, *delete_ask, *delete_yes, *delete_no, *delete_running,
      *delete_failed;
  const char *limit_five_hour, *limit_weekly, *context;
} Strings;

const Strings *strings(void);
Lang strings_lang(void);
// Shows lang everywhere and remembers it; strings_follow_system drops the
// saved choice (settings reset).
void strings_set_lang(Lang lang);
void strings_follow_system(void);

// The texts with numbers or names in them. Each writes into out.
void format_running(char *out, size_t cap, int running, int total);
void format_exit_code(char *out, size_t cap, unsigned long code);
void format_agent_mismatch(char *out, size_t cap, const char *selected, const char *actual);
void format_hooks_old_claude(char *out, size_t cap, const char *version);
void format_new_agent_in(char *out, size_t cap, const char *folder);
void format_worktree_hint(char *out, size_t cap, const char *folder);
void format_uncommitted(char *out, size_t cap, int files);
void format_more_subagents(char *out, size_t cap, int n);
void format_more(char *out, size_t cap, int n, bool above);
void format_limit_resets(char *out, size_t cap, const char *left);
void format_context_used(char *out, size_t cap, const char *folder, int percent);

// strings_time.c
// How long something has run: "42s", "2m 14s", "1h 05m".
void format_uptime(char *out, size_t cap, unsigned long long ms);
// How long ago ms was at now (both ms since 1970): "just now", "5 min.
// ago", "yesterday", "3일 전".
void format_time_ago(char *out, size_t cap, unsigned long long ms, unsigned long long now);
// Time left until a usage limit resets: "12m", "2h 13m", "3d 4h".
void format_reset_in(char *out, size_t cap, double seconds);
