// Showing the folder picker.
#include "ui/folder_picker.h"

#include <shobjidl.h>

#include "base/log.h"
#include "base/utf.h"

char *folder_picker_run(HWND owner) {
  IFileOpenDialog *dialog = NULL;
  if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog,
                              (void **)&dialog))) {
    log_warn("folders", "no folder picker");
    return NULL;
  }
  char *chosen = NULL;
  FILEOPENDIALOGOPTIONS options = 0;
  IFileOpenDialog_GetOptions(dialog, &options);
  IFileOpenDialog_SetOptions(dialog, options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
  IShellItem *item = NULL;
  if (SUCCEEDED(IFileOpenDialog_Show(dialog, owner)) && SUCCEEDED(IFileOpenDialog_GetResult(dialog, &item))) {
    wchar_t *path = NULL;
    if (SUCCEEDED(IShellItem_GetDisplayName(item, SIGDN_FILESYSPATH, &path))) {
      chosen = wide_to_utf8(path, -1, NULL);
      CoTaskMemFree(path);
    }
    IShellItem_Release(item);
  }
  IFileOpenDialog_Release(dialog);
  log_info("folders", "picked %s", chosen ? chosen : "nothing");
  return chosen;
}
