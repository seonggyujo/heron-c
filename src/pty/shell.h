// The shells offered for new panes (pwsh 7, Windows PowerShell, cmd, Git
// Bash): which are installed, their names, and how to start one.
#pragma once

#include <stdbool.h>
#include <wchar.h>

#include "base/env_block.h"

typedef enum { SHELL_PWSH, SHELL_POWERSHELL, SHELL_CMD, SHELL_GIT_BASH, SHELL_COUNT } ShellKind;

// Looks for the shells once; call before the other functions.
void shell_detect(void);
bool shell_installed(ShellKind kind);
// The first installed shell in menu order, else Windows PowerShell.
ShellKind shell_default(void);

const char *shell_name(ShellKind kind);  // shown in the UI
const char *shell_id(ShellKind kind);    // stored in settings
bool shell_from_id(const char *id, ShellKind *out);

// Full path of the program, NULL when it is not installed.
const wchar_t *shell_program(ShellKind kind);

typedef struct {
  wchar_t *command_line;
  wchar_t *environment;  // NULL: inherit unchanged
  wchar_t *cwd;
} ShellLaunch;

// How to start kind in cwd (or the user's home when cwd is not a folder),
// with env added to its environment. The shell is set up to report its
// folder with the invisible OSC 9;9 sequence before each prompt; nothing
// visible changes and no profile file is touched. Returns false when the
// shell is not installed.
bool shell_launch(ShellKind kind, const wchar_t *cwd, const EnvVar *env, int env_count, ShellLaunch *out);
void shell_launch_free(ShellLaunch *l);
