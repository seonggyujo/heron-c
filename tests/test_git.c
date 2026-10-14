// Tests for counting changes (Heron's numstat.rs tests).
#include "check.h"
#include "git/numstat.h"

void test_numstat(void) {
  GitChanges c = numstat_parse("12\t3\tsrc/a.ts\n0\t7\tdocs/b.md\n-\t-\ticon.png\n");
  CHECK(c.added == 12 && c.removed == 10 && c.files == 3);
  c = numstat_parse("");
  CHECK(c.added == 0 && c.removed == 0 && c.files == 0);
  uint64_t lines;
  CHECK(numstat_text_lines("a\nb\n", 4, &lines) && lines == 2);
  CHECK(numstat_text_lines("a\nb", 3, &lines) && lines == 2);
  CHECK(numstat_text_lines("", 0, &lines) && lines == 0);
  CHECK(!numstat_text_lines("PNG\0\x01", 5, &lines));
  numstat_add_new_file(&c, 4);
  CHECK(c.added == 4 && c.files == 1);
}
