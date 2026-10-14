// Win32 file calls behind UTF-8 paths.
#include "base/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "base/clock.h"
#include "base/mem.h"
#include "base/utf.h"

char *file_read_range(const char *path, uint64_t from, uint64_t to, size_t *len) {
  wchar_t *w = utf8_to_wide(path, -1);
  HANDLE h = CreateFileW(w, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  free(w);
  if (h == INVALID_HANDLE_VALUE) return NULL;
  LARGE_INTEGER size;
  char *data = NULL;
  if (GetFileSizeEx(h, &size)) {
    uint64_t end = to && to < (uint64_t)size.QuadPart ? to : (uint64_t)size.QuadPart;
    uint64_t start = from < end ? from : end;
    size_t n = (size_t)(end - start), got = 0;
    LARGE_INTEGER at = {.QuadPart = (LONGLONG)start};
    data = xmalloc(n + 1);
    DWORD chunk;
    SetFilePointerEx(h, at, NULL, FILE_BEGIN);
    while (got < n && ReadFile(h, data + got, (DWORD)(n - got > 1 << 20 ? 1 << 20 : n - got), &chunk, NULL) && chunk) {
      got += chunk;
    }
    data[got] = 0;
    if (len) *len = got;
  }
  CloseHandle(h);
  return data;
}

char *file_read(const char *path, size_t *len) { return file_read_range(path, 0, 0, len); }

static bool write_all(const char *path, const char *data, size_t len) {
  wchar_t *w = utf8_to_wide(path, -1);
  HANDLE h = CreateFileW(w, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  free(w);
  if (h == INVALID_HANDLE_VALUE) return false;
  DWORD wrote = 0;
  bool ok = WriteFile(h, data, (DWORD)len, &wrote, NULL) && wrote == len;
  CloseHandle(h);
  return ok;
}

bool file_write_atomic(const char *path, const char *data, size_t len) {
  char tmp[MAX_PATH * 3];
  snprintf(tmp, sizeof tmp, "%s.%lu.tmp", path, GetCurrentProcessId());
  if (!write_all(tmp, data, len)) return false;
  wchar_t *from = utf8_to_wide(tmp, -1), *to = utf8_to_wide(path, -1);
  bool ok = MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING);
  if (!ok) DeleteFileW(from);
  free(from);
  free(to);
  return ok;
}

bool file_copy(const char *from, const char *to) {
  wchar_t *a = utf8_to_wide(from, -1), *b = utf8_to_wide(to, -1);
  bool ok = CopyFileW(a, b, FALSE);
  free(a);
  free(b);
  return ok;
}

bool file_remove(const char *path) {
  wchar_t *w = utf8_to_wide(path, -1);
  bool ok = DeleteFileW(w);
  free(w);
  return ok;
}

bool file_make_dirs(const char *path) {
  char part[MAX_PATH * 3];
  snprintf(part, sizeof part, "%s", path);
  for (char *p = part + 3; *p; p++) {  // past "C:\"
    if (*p != '\\' && *p != '/') continue;
    char keep = *p;
    *p = 0;
    wchar_t *w = utf8_to_wide(part, -1);
    CreateDirectoryW(w, NULL);
    free(w);
    *p = keep;
  }
  wchar_t *w = utf8_to_wide(part, -1);
  CreateDirectoryW(w, NULL);
  free(w);
  return file_is_dir(path);
}

static bool attributes(const char *path, WIN32_FILE_ATTRIBUTE_DATA *out) {
  wchar_t *w = utf8_to_wide(path, -1);
  bool ok = GetFileAttributesExW(w, GetFileExInfoStandard, out);
  free(w);
  return ok;
}

bool file_info(const char *path, uint64_t *size, uint64_t *modified_ms) {
  WIN32_FILE_ATTRIBUTE_DATA d;
  if (!attributes(path, &d) || (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
  if (size) *size = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
  if (modified_ms) *modified_ms = clock_from_filetime(d.ftLastWriteTime.dwLowDateTime, d.ftLastWriteTime.dwHighDateTime);
  return true;
}

bool file_exists(const char *path) {
  WIN32_FILE_ATTRIBUTE_DATA d;
  return attributes(path, &d);
}

bool file_is_dir(const char *path) {
  WIN32_FILE_ATTRIBUTE_DATA d;
  return attributes(path, &d) && (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

void file_list(const char *dir, FileVisit visit, void *ctx) {
  char pattern[MAX_PATH * 3];
  file_join(pattern, sizeof pattern, dir, "*");
  wchar_t *w = utf8_to_wide(pattern, -1);
  WIN32_FIND_DATAW f;
  HANDLE h = FindFirstFileExW(w, FindExInfoBasic, &f, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
  free(w);
  if (h == INVALID_HANDLE_VALUE) return;
  do {
    if (wcscmp(f.cFileName, L".") == 0 || wcscmp(f.cFileName, L"..") == 0) continue;
    char *name = wide_to_utf8(f.cFileName, -1, NULL);
    bool more = visit(ctx, name, (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    free(name);
    if (!more) break;
  } while (FindNextFileW(h, &f));
  FindClose(h);
}

void file_join(char *out, size_t cap, const char *dir, const char *name) {
  size_t n = strlen(dir);
  bool slash = n > 0 && (dir[n - 1] == '\\' || dir[n - 1] == '/');
  snprintf(out, cap, "%s%s%s", dir, slash ? "" : "\\", name);
}
