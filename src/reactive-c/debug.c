#include <stdio.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

void _debug_level(char* title, observable_t this, int level) {
  printf("%*s: %s (%p) prop:", level, title, this->label, (void*)this);
  if(_is_disposed(this))       { printf(" disposed"); }
  if(_is_suspended(this))      { printf(" suspended"); }
  if(_is_delayed(this))        { printf(" delayed"); }
  if(!_is_propagating(this))   { printf(" no_propagation"); }
  printf("\n");
  if(this->observeds && this->observeds) {
    printf("%*.s   observing:\n", level, "");
    foreach(observable_li_t, iter, this->observeds) {
      _debug_level("   - ", iter->ob, level+3);
    }
  }
  if(_is_script(this)) {
    printf("steps:\n");
    observable_t step = this->next;
    int c = 1;
    while(step) {
      printf("%d)\n", c++);
      _debug_level("", step, level + 3);
      step = step->next;
    }
  }
}
