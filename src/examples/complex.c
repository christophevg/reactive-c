#include <stdio.h>
#include <stdlib.h>

#include "reactive-c/api.h"

#include "internals.h"

void mark(observation_t ob) {
  _mark(ob->self);
}

int main(void) {
  int _a = 0, _b = 0, _c = 0, _d = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);
  observable_t d = observe(_d);

  observable_t l11 = observe(just(b), mark, int);
  observable_t l12 = observe(just(c), mark, int);
  observable_t l13 = observe(each(c,d), mark, int);
  observable_t l14 = observe(just(d), mark, int);
  
  observable_t l21 = observe(each(a, l11), mark, int);
  observable_t l22 = observe(just(l11), mark, int);
  observable_t l23 = observe(each(b, l12), mark, int);
  observable_t l24 = observe(just(l14), mark, int);
  
  observable_t l31 = observe(each(a, l22), mark, int);
  observable_t l32 = observe(just(l23), mark, int);
  observable_t l33 = observe(each(l12, l13), mark, int);
  observable_t l34 = observe(each(d, l24), mark, int);
  
  observable_t l41 = observe(just(l31),mark, int);
  observable_t l42 = observe(each(l31,c),mark, int);
  observable_t l43 = observe(just(l32),mark, int);
  observable_t l44 = observe(just(l34),mark, int);
  observable_t l45 = observe(each(l34, l23),mark, int);
  
  observable_t l51 = observe(just(l41),mark, int);
  observable_t l52 = observe(each(l42, l44),mark, int);
  
  observable_t l61 = observe(just(l51),mark, int);
  observable_t l62 = observe(just(l42),mark, int);
  observable_t l63 = observe(each(l52, l44, l45),mark, int);
  
  observable_t l71 = observe(each(l51, l42, l62),mark, int);
  observable_t l72 = observe(each(l62, l33), mark, int);

  // to avoid warnings about unused variables
  l63 = NULL;  l43 = NULL;  l61 = NULL;  l21 = NULL;  l72 = NULL;
  
  // propagate one change through the graph
  observe_update(c);

  // dump the graph with markings
  FILE *fp = fopen("complex.dot", "w");
  to_dot(l71, fp, DOT_MESSAGE_STYLE | DOT_HIDE_LABELS | DOT_SMALL_RANK_SEP
                | DOT_SHAPE_IS_CIRCLE | DOT_SHOW_MARKED);
  fclose(fp);

  exit(EXIT_SUCCESS);
}
