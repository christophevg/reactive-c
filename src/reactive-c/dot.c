#include <stdio.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

void __to_dot(observable_t this, FILE *fp, bool show_memory, bool preamble) {
  if(_is_exported(this)) { return; }
  _mark_exported(this);

  if(preamble) {
    fprintf(fp,
      "digraph {\n"
      "  ordering=out;\n"
      "  ranksep=.4;\n"
      "  rankdir = BT;\n"
      "  node [shape=plaintext, fixedsize=true, fontsize=11, "
      "  fontname=\"Courier\", width=.25, height=.25];\n"
      "  edge [arrowsize=.6]\n"
    );
  }

  // self node
  if(show_memory) {
    fprintf(fp, "\"%p\" [label=\"%s\n%p\"", (void*)this, this->label, (void*)this);
  } else {
    fprintf(fp, "\"%p\" [label=\"%s\"", (void*)this, this->label);
  }
  // delayed/suspended observables are grey
  if(this->prop & DELAYED || this->prop & SUSPENDED) {
    fprintf(fp, " color=\"grey\", style=\"filled\"");
  }
  // values are green
  if(this->prop & VALUE) {
    fprintf(fp, " color=\"green\", style=\"filled\"");
  }
  fprintf(fp, "]\n");

  // observeds
  foreach(observable_li_t, iter, this->observeds) {
    // only generate links for observed ... not also for observers
    if(!_is_exported(iter)) {
      _mark_exported(iter);
      fprintf(fp, "\"%p\" -> \"%p\"\n", (void*)this, (void*)iter->ob);
    }
    // recurse
    __to_dot(iter->ob, fp, show_memory, false);
  }

  // sequential relationships (e.g. scripts' steps)
  if(this->next) {
    fprintf(fp, "\"%p\" -> \"%p\" [style=\"dotted\"]\n", (void*)this, (void*)this->next);
    fprintf(fp, "{ rank = same; \"%p\" \"%p\" }", (void*)this, (void*)this->next);
  }

  // recurse observers
  foreach(observable_li_t, iter, this->observers) {
    __to_dot(iter->ob, fp, show_memory, false);
  }

  // recurse parent
  if(this->parent) {
    __to_dot(this->parent, fp, show_memory, false);
  }

  if(preamble) { fprintf(fp, "}\n"); }
}
