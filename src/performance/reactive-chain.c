#include <stdlib.h>
#include <stdio.h>

#include "lib/time.h"

#include "reactive-c/api.h"

void id(observation_t ob) {
  *((int*)ob->observer) = *((int*)ob->observeds[0]);
}

#define CHAIN_LENGTH     100
#define UPDATES      5000000
#define ITERATIONS         5

int main(void) {
  // source
  int _a = 0;
  observable_t source = observe(_a);
  
  // now chain 100 observers
  observable_t tail = source;
  for(int i=0; i<CHAIN_LENGTH; i++) {
    tail = observe(just(tail), id, int);
  }
  
  int total = 0;

  for(int iteration=0; iteration<ITERATIONS; iteration++) {
    struct timespec start = now();
  
    // measure time to perform updates
    for(int i=0; i<UPDATES; i++) {
      set(int, source, i);
    }
  
    double d = diff(start, now());
  
    printf("duration = %f sec\n", d);

    int updates = (int)(UPDATES / d);
    printf("%d updates/sec\n", updates);
    
    total += updates;
  }
  
  printf("avg: %d updates/sec\n", (int)(total / (ITERATIONS*1.0)));
  
  exit(EXIT_SUCCESS);
}
