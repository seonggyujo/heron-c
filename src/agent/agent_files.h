// The files agents write for a pane (Heron's agent/files.rs) and the two
// variables every shell gets so they know where:
//   HERON_PANE       the pane's key, "<heron-c pid>-<pane id>"
//   HERON_AGENT_DIR  the folder to write into
// heron-c's own hooks (hooks/hook_entry.c) write <key>.state.json,
// <key>.model.json and one <key>.sub.<id>.json per running subagent; the
// heron-limits plugin's status line writes <key>.status.json. The names
// and formats are Heron's, so the plugin works unchanged; the folder is
// heron-c's own.
#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum { AGENT_FILE_STATUS, AGENT_FILE_STATE, AGENT_FILE_MODEL, AGENT_FILE_KINDS } AgentFileKind;

#define AGENT_KEY_MAX 32
#define AGENT_PATH_MAX 1024

// %LOCALAPPDATA%\heron-c\agents, made on first use (UTF-8).
const char *agent_dir(void);
void agent_key(int pane, char out[AGENT_KEY_MAX]);
// A key as agent_key makes it, so a file name built from it stays in dir.
bool agent_is_key(const char *text);
// Letters, digits, - and _ only, at most 64.
bool agent_is_subagent_id(const char *text);

void agent_file_path(char out[AGENT_PATH_MAX], const char *dir, const char *key, AgentFileKind kind);
void agent_subagent_path(char out[AGENT_PATH_MAX], const char *dir, const char *key, const char *id);

// Calls fn with the id and path of each subagent file of key.
typedef void (*SubagentVisit)(void *ctx, const char *id, const char *path);
void agent_subagent_files(const char *dir, const char *key, SubagentVisit fn, void *ctx);
void agent_remove_subagents(const char *dir, const char *key);
// The pane closed: its files go.
void agent_remove_pane(const char *dir, int pane);
// Files older than a day are leftovers of a run that did not clean up.
void agent_remove_stale(const char *dir);
