// OSC 8 hyperlinks: the program marks characters as a link to a URI. URIs
// are kept in a table so a cell carries only a 16-bit index; cells in
// history keep pointing at their URI.
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"
#include "vt/term_internal.h"

#define MAX_LINKS 65535

static uint16_t link_id(Term *t, const char *uri, size_t len) {
  for (int i = 0; i < t->link_count; i++) {
    if (strlen(t->links[i]) == len && memcmp(t->links[i], uri, len) == 0) return (uint16_t)(i + 1);
  }
  if (t->link_count == MAX_LINKS) return 0;
  if (t->link_count == t->link_cap) {
    t->link_cap = t->link_cap ? t->link_cap * 2 : 16;
    t->links = xrealloc(t->links, sizeof(char *) * (size_t)t->link_cap);
  }
  char *copy = xmalloc(len + 1);
  memcpy(copy, uri, len);
  copy[len] = 0;
  t->links[t->link_count++] = copy;
  return (uint16_t)t->link_count;
}

void term_hyperlink(Term *t, const char *text, size_t len) {
  const char *semi = memchr(text, ';', len);  // after the params
  if (!semi) return;
  const char *uri = semi + 1;
  size_t uri_len = len - (size_t)(uri - text);
  t->pen.link = uri_len ? link_id(t, uri, uri_len) : 0;
}

const char *term_link_uri(const Term *t, uint16_t id) {
  return id > 0 && id <= t->link_count ? t->links[id - 1] : NULL;
}

void term_free_links(Term *t) {
  for (int i = 0; i < t->link_count; i++) free(t->links[i]);
  free(t->links);
  t->links = NULL;
  t->link_count = t->link_cap = 0;
}
