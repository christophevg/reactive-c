#include <stdio.h>
#include <stdbool.h>

#include "observable.h"
#include "observables.h"
#include "internals.h"

#include "iterator.h"

#include "dot.h"

void __to_dot(observable_t this, FILE *fp, int flags, bool preamble) {
  if(_is_exported(this)) { return; }
  _mark_exported(this);
  
  bool show_messages   = flags & DOT_MESSAGE_STYLE;
  bool show_memory     = flags & DOT_SHOW_MEMORY;
  bool show_label      = !(flags & DOT_HIDE_LABELS);
  bool small_rank_sep  = flags & DOT_SMALL_RANK_SEP;
  bool shape_is_circle = flags & DOT_SHAPE_IS_CIRCLE;

  if(preamble) {
    fprintf(fp,
      "digraph {\n"
      "  ordering=out;\n"
      "  ranksep=.%d;\n"
      "  rankdir = %s;\n"
      "  node [shape=%s, fixedsize=true, fontsize=11, "
      "  fontname=\"Courier\", width=.25, height=.25];\n"
      "  edge [arrowsize=.6]\n",
      small_rank_sep ? 2 : 4,
      show_messages ? "TB" : "BT",
      shape_is_circle ? "circle" : "rectangle"
    );
  }

  // self node
  fprintf(fp, "subgraph level%d { rank=same; \"%p\" [label=\"%s",
          _is_script_part(this) ? 666 : this->level, (void*)this,
          show_label ? this->label : "");
  if(show_memory) { fprintf(fp, "\n%p",(void*)this);     }
  fprintf(fp, "\"");

  // delayed/suspended observables are grey
  if(this->prop & DELAYED || this->prop & SUSPENDED) {
    fprintf(fp, " color=\"grey\", style=\"filled\"");
  }
  // values are green
  if(this->prop & VALUE) {
    fprintf(fp, " color=\"green\", style=\"filled\"");
  }
  fprintf(fp, "] }\n");

  // observeds
  foreach(observable_li_t, iter, this->observeds) {
    // only generate links for observed ... not also for observers
    if(!_is_exported(iter)) {
      _mark_exported(iter);
      fprintf(fp, "\"%p\" -> \"%p\"%s\n",
             (void*)this, (void*)iter->ob, show_messages ? " [dir=back]": "" );
    }
    // recurse
    __to_dot(iter->ob, fp, flags, false);
  }

  // sequential relationships (e.g. scripts' steps)
  if(this->next) {
    fprintf(fp, "\"%p\" -> \"%p\" [style=\"dotted\"]\n",
            (void*)this, (void*)this->next);
    fprintf(fp, "{ rank = same; \"%p\" \"%p\" }",
            (void*)this, (void*)this->next);
  }

  // recurse observers
  foreach(observable_li_t, iter, this->observers) {
    __to_dot(iter->ob, fp, flags, false);
  }

  // recurse parent
  if(this->parent) {
    __to_dot(this->parent, fp, flags, false);
  }

  if(preamble) { fprintf(fp, "}\n"); }
}
