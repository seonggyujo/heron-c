// The IDropTarget COM object, written out in C: a vtable of the seven
// methods and one static object per window.
#include "ui/file_drop.h"

#include <ole2.h>
#include <shellapi.h>

#include "ui/file_drop_paths.h"

typedef struct {
  IDropTarget iface;  // first, so an IDropTarget * is a DropTarget *
  App *app;
  bool accept;  // the drag carries files
} DropTarget;

static DropTarget g_target;

static HRESULT STDMETHODCALLTYPE query(IDropTarget *self, REFIID riid, void **out) {
  if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropTarget)) {
    *out = self;
    return S_OK;
  }
  *out = NULL;
  return E_NOINTERFACE;
}

// The object lives as long as the program, so counting is not needed.
static ULONG STDMETHODCALLTYPE add_ref(IDropTarget *self) { return 1; }
static ULONG STDMETHODCALLTYPE release(IDropTarget *self) { return 1; }

static FORMATETC files_format(void) {
  FORMATETC f = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  return f;
}

// Marks the pane under the screen point; returns the effect to show.
static DWORD mark(DropTarget *t, POINTL pt) {
  App *a = t->app;
  POINT p = {pt.x, pt.y};
  ScreenToClient(a->hwnd, &p);
  Pane *pane = t->accept ? panes_at(&a->panes, p.x, p.y) : NULL;
  int id = pane ? pane->id : 0;
  if (id != a->drop_pane) {
    a->drop_pane = id;
    app_redraw(a);
  }
  return id ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

static HRESULT STDMETHODCALLTYPE drag_enter(IDropTarget *self, IDataObject *data, DWORD keys, POINTL pt,
                                            DWORD *effect) {
  DropTarget *t = (DropTarget *)self;
  FORMATETC f = files_format();
  t->accept = IDataObject_QueryGetData(data, &f) == S_OK;
  *effect = mark(t, pt);
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drag_over(IDropTarget *self, DWORD keys, POINTL pt, DWORD *effect) {
  *effect = mark((DropTarget *)self, pt);
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drag_leave(IDropTarget *self) {
  POINTL nowhere = {-100000, -100000};
  mark((DropTarget *)self, nowhere);
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drop(IDropTarget *self, IDataObject *data, DWORD keys, POINTL pt,
                                      DWORD *effect) {
  DropTarget *t = (DropTarget *)self;
  *effect = mark(t, pt);
  Pane *pane = panes_find(&t->app->panes, t->app->drop_pane);
  FORMATETC f = files_format();
  STGMEDIUM medium;
  if (pane && IDataObject_GetData(data, &f, &medium) == S_OK) {
    drop_paths_paste(t->app, pane, (HDROP)medium.hGlobal);
    ReleaseStgMedium(&medium);
  }
  drag_leave(self);
  return S_OK;
}

static IDropTargetVtbl g_vtbl = {query, add_ref, release, drag_enter, drag_over, drag_leave, drop};

void file_drop_register(App *a) {
  g_target.iface.lpVtbl = &g_vtbl;
  g_target.app = a;
  RegisterDragDrop(a->hwnd, &g_target.iface);
}

void file_drop_revoke(App *a) { RevokeDragDrop(a->hwnd); }
