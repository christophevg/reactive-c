#include <stdio.h>
#include <stdlib.h>

#include "reactive-c/api.h"

void dummy(observation_t _) {}

int main(void) {
  int _a = 0, _b = 0, _c = 0, _d = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);
  observable_t d = observe(_d);

  observable_t l11 = observe(just(b), dummy, void);
  observable_t l12 = observe(just(c), dummy, void);
  observable_t l13 = observe(each(c,d), dummy, void);
  observable_t l14 = observe(just(d), dummy, void);
  
  observable_t l21 = observe(each(a, l11), dummy, void);
  observable_t l22 = observe(just(l11), dummy, void);
  observable_t l23 = observe(each(b, l12), dummy, void);
  observable_t l24 = observe(just(l14), dummy, void);
  
  observable_t l31 = observe(each(a, l22), dummy, void);
  observable_t l32 = observe(just(l23), dummy, void);
  observable_t l33 = observe(each(l12, l13), dummy, void);
  observable_t l34 = observe(each(d, l24), dummy, void);
  
  observable_t l41 = observe(just(l31),dummy, void);
  observable_t l42 = observe(each(l31,c),dummy, void);
  observable_t l43 = observe(just(l32),dummy, void);
  observable_t l44 = observe(just(l34),dummy, void);
  observable_t l45 = observe(each(l34, l23),dummy, void);
  
  observable_t l51 = observe(just(l41),dummy, void);
  observable_t l52 = observe(each(l42, l44),dummy, void);
  
  observable_t l61 = observe(just(l51),dummy, void);
  observable_t l62 = observe(just(l42),dummy, void);
  observable_t l63 = observe(each(l52, l44, l45),dummy, void);
  
  observable_t l71 = observe(each(l51, l42, l62),dummy, void);
  observable_t l72 = observe(each(l62, l33), dummy, void);

  FILE *fp = fopen("complex.dot", "w");
  to_dot(l71, fp, DOT_MESSAGE_STYLE | DOT_HIDE_LABELS | DOT_SMALL_RANK_SEP
                | DOT_SHAPE_IS_CIRCLE);
  fclose(fp);

  exit(EXIT_SUCCESS);
}
