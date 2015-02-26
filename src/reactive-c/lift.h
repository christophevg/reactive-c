#ifndef __LIFT_H
#define __LIFT_H

// demo for lifting through macro-expansion
#define lift2(type, fun) \
  void __lifted_##fun(observation_t ob) { \
    *(type*)(ob->observer) = fun((*(type*)(ob->observeds[0])), (*(type*)(ob->observeds[1]))); \
  }

#define lifted(x) __lifted_##x

#endif
