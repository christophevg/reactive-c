// test

#include <stdlib.h>
#include <stdio.h>

void update(void *a, void *b) {
  (*(double*)(b)) = ((*(double*)(a)) + 10);
}

int main(void) {
  
  double a = 12.0;
  double b = 13.0;

  printf("a=%f | b=%f\n", a, b);
  
  update((void*)&a, (void*)&b);
  
  printf("a=%f | b=%f\n", a, b);
  
  exit(EXIT_SUCCESS);
}