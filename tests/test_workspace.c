// Tests for saving and rebuilding an arrangement and the saved work.
#include <stdlib.h>
#include <string.h>

#include "app/workspace.h"
#include "check.h"
#include "layout/arrangement.h"
#include "layout/tree_shape.h"

static Json *parse(const char *text) { return json_parse(text, strlen(text)); }

void test_tree_shape(void) {
  Arrangement a = {0};
  int live[3] = {11, 12, 13};
  arrange_tidy(&a, live, 3);
  CHECK(arrange_shape(&a, live, 3) == NULL);  // the automatic grid is not saved
  arrange_move(&a, 13, 11, SIDE_LEFT);
  Json *shape = arrange_shape(&a, live, 3);
  Buf text = {0};
  json_write(shape, &text, false);
  buf_append(&text, "", 1);
  CHECK_STR(text.data, "{\"kind\":\"split\",\"axis\":\"row\",\"children\":[{\"kind\":\"leaf\",\"item\":2},"
                       "{\"kind\":\"leaf\",\"item\":0},{\"kind\":\"leaf\",\"item\":1}],\"sizes\":[0.25,0.25,0.5]}");
  buf_free(&text);
  Arrangement b = {0};
  int reopened[3] = {21, 22, 23};
  CHECK(arrange_restore(&b, shape, reopened, 3) && b.manual);
  char out[64];
  tree_describe(b.root, out, sizeof out);
  CHECK_STR(out, "row(23 21 22)");
  CHECK(!arrange_restore(&b, shape, reopened, 2));  // a pane is missing
  Json *bad = parse("{\"kind\":\"split\",\"axis\":\"row\",\"children\":[{\"kind\":\"leaf\",\"item\":0},"
                    "{\"kind\":\"leaf\",\"item\":0}],\"sizes\":[0.5,0.5]}");
  CHECK(tree_from_shape(bad, reopened, 2) == NULL);  // the same pane twice
  json_free(bad);
  bad = parse("{\"kind\":\"split\",\"axis\":\"row\",\"children\":[{\"kind\":\"leaf\",\"item\":0},"
              "{\"kind\":\"leaf\",\"item\":1}],\"sizes\":[0.5,0.6]}");
  CHECK(tree_from_shape(bad, reopened, 2) == NULL);  // sizes not adding up to 1
  json_free(bad);
  json_free(shape);
  arrange_free(&a);
  arrange_free(&b);
}

void test_saved_work(void) {
  SavedPane panes[2] = {{"pwsh", "C:\\work", "158ec1a7-965f-43a3-99cb-8f9e257cb741"}, {"cmd", "", ""}};
  Json *layout = parse("{\"kind\":\"leaf\",\"item\":0}");
  char *text = workspace_text(panes, 2, layout);
  CHECK_STR(text, "{\"panes\":[{\"shell\":\"pwsh\",\"cwd\":\"C:\\\\work\",\"session\":"
                  "\"158ec1a7-965f-43a3-99cb-8f9e257cb741\"},{\"shell\":\"cmd\",\"cwd\":null,\"session\":null}],"
                  "\"layout\":{\"kind\":\"leaf\",\"item\":0}}");
  SavedWorkspace w;
  CHECK(workspace_parse(text, &w) && w.count == 2 && strcmp(w.panes[0].cwd, "C:\\work") == 0);
  CHECK(w.panes[1].session[0] == 0 && json_equal(w.layout, layout));
  workspace_free(&w);
  CHECK(!workspace_parse("{\"panes\":3}", &w) && !workspace_parse("not json", &w));
  free(text);
  json_free(layout);
}
