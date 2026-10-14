// Files and folders by UTF-8 path. Failures return false or NULL and are
// left to the caller to log; nothing here retries.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// The whole file, or the part from byte `from` up to `to` (to = 0: the
// end), malloc'd with a NUL after it. len (optional) gets the byte count.
char *file_read(const char *path, size_t *len);
char *file_read_range(const char *path, uint64_t from, uint64_t to, size_t *len);
// Writes a temporary file beside path and renames it over path, so a
// reader never sees half a file.
bool file_write_atomic(const char *path, const char *data, size_t len);
bool file_copy(const char *from, const char *to);
bool file_remove(const char *path);
bool file_make_dirs(const char *path);

// Size and last write (ms since 1970); false when there is no such file.
bool file_info(const char *path, uint64_t *size, uint64_t *modified_ms);
bool file_exists(const char *path);
bool file_is_dir(const char *path);

// Calls visit with the name of each entry of dir (not . or ..) until it
// returns false.
typedef bool (*FileVisit)(void *ctx, const char *name, bool is_dir);
void file_list(const char *dir, FileVisit visit, void *ctx);

// dir + "\" + name into out.
void file_join(char *out, size_t cap, const char *dir, const char *name);
