// Small path helpers shared by the panes and the agent sidebar (Heron's
// app/paths.ts). Pure.
#pragma once

#include <stdbool.h>
#include <stddef.h>

// Last folder of a Windows or Unix path: "C:\Users\me\app" → "app",
// "C:\" → "C:". Empty for an empty path.
void folder_name(const char *path, char *out, size_t cap);
// Inside a worktree of `claude --worktree`: <repo>\.claude\worktrees\<name>\...
bool in_worktree(const char *path);
// The name to show for a folder: "heron", or "heron · fix-login" for that
// worktree of heron.
void folder_label(const char *path, char *out, size_t cap);
// The key a folder is grouped and remembered by: lowercase, without
// trailing slashes (Windows folders differ only in those are one).
void folder_key(const char *path, char *out, size_t cap);
