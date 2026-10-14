// Adding, removing and finding our entries in settings.json `hooks`.
#include "hooks/hook_config.h"

#include <ctype.h>
#include <string.h>

// The events, with the matcher of each. The `args` form needs Claude Code
// 2.1.139 (hooks.c checks it). All exist since then but PostModelSwitch
// (2.1.251); an older Claude Code skips an event name it does not know.
static const struct {
  const char *event, *matcher;
} EVENTS[] = {
    // Not "compact": a compaction can happen in the middle of a turn.
    {"SessionStart", "startup|resume|clear"},
    {"UserPromptSubmit", NULL},
    {"PermissionRequest", NULL},
    {"Notification", "permission_prompt|idle_prompt|elicitation_dialog"},
    {"Stop", NULL},
    {"StopFailure", NULL},
    {"SessionEnd", NULL},
    {"PostModelSwitch", NULL},
    {"SubagentStart", NULL},
    {"SubagentStop", NULL},
};
#define EVENT_COUNT (int)(sizeof EVENTS / sizeof *EVENTS)

// The hook exits at once; this only bounds a stuck one.
#define TIMEOUT_SECONDS 10

static bool ends_with_exe(const char *command) {
  static const char NAME[] = "heron-c.exe";
  size_t n = strlen(command), k = sizeof NAME - 1;
  if (n < k) return false;
  for (size_t i = 0; i < k; i++) {
    if (tolower((unsigned char)command[n - k + i]) != NAME[i]) return false;
  }
  return true;
}

static bool is_ours(const Json *hook) {
  const Json *args = json_get(hook, "args");
  const char *command = json_string(json_get(hook, "command"));
  bool flag = json_is(args, JSON_ARRAY) && args->count == 1 && json_string(args->items[0]) &&
              strcmp(json_string(args->items[0]), HOOK_FLAG) == 0;
  return flag && command && ends_with_exe(command);
}

// Our entries for event: how many, and whether each runs exe.
static int ours(const Json *hooks, const char *event, const char *exe, bool *all_exe) {
  const Json *groups = json_get(hooks, event);
  int n = 0;
  for (int g = 0; json_is(groups, JSON_ARRAY) && g < groups->count; g++) {
    const Json *list = json_get(groups->items[g], "hooks");
    for (int i = 0; json_is(list, JSON_ARRAY) && i < list->count; i++) {
      if (!is_ours(list->items[i])) continue;
      n++;
      const char *command = json_string(json_get(list->items[i], "command"));
      if (!command || strcmp(command, exe) != 0) *all_exe = false;
    }
  }
  return n;
}

HookPresence hook_config_presence(const Json *settings, const char *exe) {
  const Json *hooks = json_get(settings, "hooks");
  bool any = false, current = true;
  for (int e = 0; e < EVENT_COUNT; e++) {
    bool all_exe = true;
    int n = ours(hooks, EVENTS[e].event, exe, &all_exe);
    any |= n > 0;
    current &= n == 1 && all_exe;
  }
  return !any ? HOOKS_ABSENT : current ? HOOKS_CURRENT : HOOKS_STALE;
}

static Json *entry(const char *matcher, const char *exe) {
  Json *hook = json_new(JSON_OBJECT), *args = json_new(JSON_ARRAY), *list = json_new(JSON_ARRAY);
  json_set(hook, "type", json_new_string("command"));
  json_set(hook, "command", json_new_string(exe));
  json_push(args, json_new_string(HOOK_FLAG));
  json_set(hook, "args", args);
  json_set(hook, "timeout", json_new_number(TIMEOUT_SECONDS));
  json_push(list, hook);
  Json *group = json_new(JSON_OBJECT);
  if (matcher) json_set(group, "matcher", json_new_string(matcher));
  json_set(group, "hooks", list);
  return group;
}

// The value of key in object, made with type when missing; NULL when it
// is there with another type.
static Json *child(Json *object, const char *key, JsonType type) {
  Json *v = json_get(object, key);
  if (!v) {
    v = json_new(type);
    json_set(object, key, v);
  }
  return json_is(v, type) ? v : NULL;
}

bool hook_config_add(Json *settings, const char *exe) {
  hook_config_remove(settings);
  if (!json_is(settings, JSON_OBJECT)) return false;
  Json *hooks = child(settings, "hooks", JSON_OBJECT);
  if (!hooks) return false;
  for (int e = 0; e < EVENT_COUNT; e++) {
    Json *groups = child(hooks, EVENTS[e].event, JSON_ARRAY);
    if (!groups) return false;
    json_push(groups, entry(EVENTS[e].matcher, exe));
  }
  return true;
}

// Drops our hooks from one event's groups, and the groups only they filled.
static bool remove_from(Json *groups) {
  bool changed = false;
  for (int g = groups->count - 1; g >= 0; g--) {
    Json *list = json_get(groups->items[g], "hooks");
    if (!json_is(list, JSON_ARRAY)) continue;
    int before = list->count;
    for (int i = list->count - 1; i >= 0; i--) {
      if (is_ours(list->items[i])) json_remove_at(list, i);
    }
    changed |= list->count != before;
    if (list->count == 0 && before > 0) json_remove_at(groups, g);
  }
  return changed;
}

bool hook_config_remove(Json *settings) {
  Json *hooks = json_get(settings, "hooks");
  if (!json_is(hooks, JSON_OBJECT)) return false;
  bool changed = false;
  for (int e = 0; e < hooks->count; e++) {
    if (json_is(hooks->items[e], JSON_ARRAY)) changed |= remove_from(hooks->items[e]);
  }
  if (!changed) return false;
  for (int e = hooks->count - 1; e >= 0; e--) {
    if (json_is(hooks->items[e], JSON_ARRAY) && hooks->items[e]->count == 0) json_remove_at(hooks, e);
  }
  if (hooks->count == 0) json_remove(settings, "hooks");
  return true;
}
