#include <stdio.h>
#include <stdlib.h>

#include "reactive-c/api.h"

#include "internals.h"

void add_one(observation_t ob) {
  (*(int*)ob->observer) = (*(int*)(ob->observeds[0])) + 1;
}

void times_three(observation_t ob) {
  (*(int*)ob->observer) = (*(int*)(ob->observeds[0])) * 3;
}

void sum(observation_t ob) {
  (*(int*)ob->observer) =
    (*(int*)(ob->observeds[0])) + (*(int*)(ob->observeds[1]));
}

// this example shows what is considered a (possible) glitch

//     b (=a+1)
//   /          \
// a              d (=b+c)
//   \          /
//     c (=a*3)

// if a=5 => b=6, c=15, d=61

int main(void) {
  int _a = 0;
  
  observable_t a = observe(_a);

  observable_t b = observe(just(a),    add_one,     int);
  observable_t c = observe(just(a),    times_three, int);
  observable_t d = observe(each(b, c), sum,         int);
  
  // propagate one change through the graph
  set(a, 5);
  
  printf("a=%d, b=%d, c=%d, d=%d\n",
         *(int*)observable_value(a),
         *(int*)observable_value(b),
         *(int*)observable_value(c),
         *(int*)observable_value(d));
  
  FILE *fp = fopen("glitch.dot", "w");
  to_dot(a, fp);
  fclose(fp);

  exit(EXIT_SUCCESS);
}
