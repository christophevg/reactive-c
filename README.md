# Reactive C

An experiment on implementing a reactive programming (RP) API in pure C.  
Christophe VG (<contact@christophe.vg>)  
[https://github.com/christophevg/reactive-c](https://github.com/christophevg/reactive-c)

## Introduction

The origin of this experiment is the paper titled "[Deprecating the Observer Pattern with Scale.React](http://infoscience.epfl.ch/record/176887/files/DeprecatingObservers2012.pdf)". The goal of this experiment is to implement a RP API in C, thus making it available to e.g. embedded development in C.

### What do I consider RP?

The core of RP is an extended implementation of the observer pattern, or as I like to call it "_The Observer Pattern on Steroids_". There is no black magic in reactive programming, just a nice(r) API to construct and compose observers and observables. (IHMO)

### Why embedded?

Although RP typically focuses on UI development and proposes a solution to its (so called) callback hell, it is not difficult to see a strong link to embedded development. In stead of user interaction, the events that require observation now are changes in (digital and analogue) signals on IO ports,...

Being able to deal with these changes in a more natural way, might also prove to be beneficial to embedded development... if the incurred cost is acceptable.

### The Million $ Question

The questions this experiment wants to answer are "_Can the concepts of RP be applied to pure C programming? And is it worth the hassle?_ "

### Approach

Using different examples, some of the RP concepts are implemented in pure C. Given the nature of C, some compromises have to be made and certain aspects of RP require creative solutions. At the end, a more elaborate example will be implemented and evaluated against an implementation using "basic" observers.

## Examples

Each of the following examples introduces an aspect of RP and its implementation in the context of Reactive C.

### Observed Values (temperature.c)

The first concept is that of **observed values**. Given a value (read: variable) representing e.g. a temperature, we can turn it in an observable using the `observe` constructor. As there is no black magic, updates to the underlying variable need to be triggered using `observe_update`.

The following example demonstrates how to implement an observable temperature value. This can be implemented in a module of its own, hiding the internal temperature representation, only exposing the `observable_temp` observable.

```c
double temp = 123;

observable_t observable_temp;

void temp_init() {
  observable_temp = observe(temp);
}

void temp_update(double update) {
  temp = update;
  observe_update(observable_temp);
}
```

Given this basic observable, we can now observe it using an observer (function). We again use the `observe` constructor. This time we don't provide a reference to a variable, but provide it a list of observables, an observer function and the type of the output of that observer (function).

**Compromise #1**: C is statically typed. This means that an observer function also needs to be statically typed. To allow any kind of arguments and result, we need to resort to `void*` (which we also define as `unknown_t`) and **explicit casts** whenever we want to access these values.

An observer (function) has to adhere to the `observer_t` type, which is defined as `typedef void (*observer_t)(observation_t);`. Such an `observation_t` consists of components:

1. an array of `unknown_t` arguments, used by the observer (function) as input. 
2. a pointer to a single `unknown_t` result variable to hold the output of the observer (function).

**Compromise #2**: The output value also needs to be stored in memory somehow. Again, because C is statically typed and it provides no ways to access the type of a function through reflection, we also need to explicitly provide it to the `observe` constructor.

```c
void c2f(observation_t ob) {
  (*(double*)(ob->observer)) = ( (*(double*)(ob->observeds[0])) * 1.8 ) + 32;
}

int main(void) {
  observable_temp_init();

  observable_t temp_f = observe(just(observable_temp), c2f, double);
  ...
}
```
Note that observing some observable value, using an observer (function) also implies the creation of a new observable. Whenever the observed value changes, the observer (function) will be executed, resulting in an updated value. This way it becomes an observable. So we now can also observe `temp_f`, which is the temperature converted to Fahrenheit.

```c
void display(observable_t ob) {
  printf( "observable was updated to %fC/%fF\n",
          *(double*)(ob->observed[0]), *(double*)(ob->observed[1]) );
}
observable_t displayer = observe(both(observable_temp, temp_f), display);
```

Again we apply the `observe` constructor, now to observe both the `observable_temp` and `temp_f` observables. Notice the use of the helper constructors `just` and `both` to turn arbitrary lists of arguments in an array (technically a linked list) and pass it as the first argument to the `observe` constructor.

**Note**: Besides `just`, which accepts a single argument, and `both` which accepts two arguments, the generic `each` accepts a variable number of arguments. The former are macro's transforming them in calls to `each`.

**Note**: The `observe` constructor is in fact a collection of macro's. Depending on the number of arguments, it actually calls different `observe` implementations. Given a single argument, the argument is treated as a _value_. Because _every_ value must be referenced and cast to `void*` or `unknown_t`, the macro expansion will take care of this, allowing a clean call to `observe` simply passing the variable name.  
With two arguments, the first argument is a _list of observables_ and the second an _observer_ (function) _that doesn't produce output_. Adding a third argument defines the _type_ of the output produced by the observer (function). Finally, a fourth argument can be used to turn the output _type_ in an array of the given _size_.

When we update the initial temperature, the two observers will be triggered and will compute the temperature in Fahrenheit and display both values.

```c
temp_update(19);
// output: observable was updated to 19.000000C/66.200000F
```

Although simple at first sight, this construction already introduces an important problem with observers: **glitches**. To understand this problem, we need to introduce the concept of a **dependency graph**. Three observables depend on each other: `temp_f` depends on `observable_temp` and `display` on both of them. We can visualize these dependencies in graph:

```
                level
observable_temp   0    <-- update
    /     \
 temp_f    |      1
    \     /
   displayer      2    --> printf

```
A glitch can appear if we don't take these dependencies into account when propagating an _update_ through the graph. The `displayer` observer can only be updated when both observables that it depends on are updated.

To avoid such a glitch, the concept of **levels** is introduced, internally. A level is a numeric property of an observable, indicating its depth in the dependency graph. Updates are propagated through the dependency graph one level at a time, guaranteeing that all observed observables are consistent with respect to the update.

One final basic operation is defined for observables: `dispose`. This allows the destruction of an observable, removing it from the dependency graph.

```c
temp_update(19); // output: observable was updated to 19.000000C/66.200000F

dispose(displayer);

temp_update(22); // no output
```

### Merging (merging.c, map.c)

Given the basic concept of observables, we can define operations to combine them. Using `merge` we can take several observables and combine them into a new observable that will propagate each change to those observables.

```c
void display(observable_t ob) {
  printf("current value = value: %f\n", *(double*)(ob->observed[0]));
}

int main(void) {
  double _a, _b, _c;

  observable_t a = observe(_a);
  observable_t b = observe(_b);
  observable_t c = observe(_c);

  observable_t abc = merge(a, b, c);

  observe(just(abc), display);

  _a = 1;  observe_update(a);  // output: current value = value: 1.000000.
  _b = 2;  observe_update(b);  // output: current value = value: 2.000000.
  _c = 3;  observe_update(c);  // output: current value = value: 3.000000.
  ...
}
```

A second operation is `map`. This takes an observable and a function and applies the function to every observed update. A typical example usage is (type) conversion.

```c
void double2string(observation_t ob) {
  snprintf(((char*)ob->observer), 10, "%0.f", *(double*)(ob-observed[0]));
}

void display(observation_t ob) {
  printf("current value = value: %s.\n", (char*)(ob->observed[0]));
}

int main(void) {
  double _a;
  
  observable_t a = observe(_a);
  observable_t A = map(a, double2string, char, 10);
  observe(just(A), display);

  _a = 1;  observe_update(a);  // output: current value = value: 1.  
  _a = 2;  observe_update(a);  // output: current value = value: 2.  
  _a = 3;  observe_update(a);  // output: current value = value: 3.
  ...
}
```
Notice that `map(a, double2string, char, 10);` is _merely_ a wrapper around the basic `observe` constructor and is expanded to `observe(just(a), double2string, char, 10);`

### Lifting Functions (add.c, lift.c)

Another important aspect of RP is the ability to apply functions to observables. Ideally this should be transparent, but here we also are bound to the static typing of C and need to introduce **compromise #3**: explicit **lifting**.

Lifting takes a function and allows it to function on observables. Let's first look at how we can do this manually: We can't overload operators in C, but we can fall back on functions to perform these operations. Let's take `+` as an example and implement `add`. As we can't have the same function to deal with `int` and `double`, we also need separate functions for each of them, let's call them `addi` and `addd`. Implementing `addi` using Reactive C so far allows us to define a _lifted_ function as such:

```c
void _addi(observation_t ob) {
  (*(int*)ob->observer) = (*(int*)(ob->observed[0])) + (*(int*)(ob->observed[1]));
}

observable_t addi(observable_t a, observable_t b) {
  return observe(each(a, b), _addi, int);
}

int main(void) {
  int _var1, _var2;
  
  observable_t var1 = observe(_var1);
  observable_t var2 = observe(_var2);
  
  observable_t var3 = addi(var1, var2);

  _var1 = 1;  observe_update(var1);
  _var2 = 2;  observe_update(var2);

  printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));
  // output: 1 + 2 = 3
  ...
 }
```
**Note**: The current value of an observable can be extracted using the `observable_value` function.

We can also add some more syntactic sugar and use some generic lifting macro to create the lifted function for us, given a basic function that doesn't know about Reactive C:

```c
int add(int a, int b) {
  return a + b;
}

lift2(int, add);

int main(void) {
  int _var1, _var2;
  
  observable_t var1 = observe(_var1);
  observable_t var2 = observe(_var2);
  
  observable_t var3 = observe(both(var1, var2), lifted(add), int);

  _var1 = 1;  observe_update(var1);
  _var2 = 2;  observe_update(var2);

  printf("%d + %d = %d\n", _var1, _var2, *(int*)observable_value(var3));
  // output: 1 + 2 = 3
  ...
}
```

Macro `lift2` will in fact expand in this case to:

```c
void __lifted_add(observation_t ob) {
   *(int*)(ob->observer) = add((*(int*)(ob->observed[0])), (*(int*)(ob-served[1])));
}
```

The helper macro `lifted` applied to `add` will expand to the `__lifted_add`, hiding the internal expansions.

### Scripts (await.c)

One of the objections made by RP is that of the inherent inversion of control effect of callbacks. To resolve this, an imperative, internal DSL is (often) proposed. Reactive C implements this concept using **scripts**.

```c
int main(void) {
  int _a = 0, _b = 0;
  
  observable_t a = observe(_a);
  observable_t b = observe(_b);
  
  run(
    script(
      await(a),
      await(b),
      await(a)
    )
  );
  
  _b = 1; observe_update(b);  // does nothing
  _a = 1; observe_update(a);  // finalizes await(a)
  _a = 2; observe_update(a);  // does nothing
  _b = 2; observe_update(b);  // finalizes await(b)
  _b = 3; observe_update(b);  // does nothing
  _a = 3; observe_update(a);  // finalizes await(a)
  _a = 4; observe_update(a);  // does nothing
  _b = 4; observe_update(b);  // does nothing
  ...  
}
```

A script consists of **suspended observables**, representing a statement from the DSL. A script must be `run`, before it actually executes the defined statements, by activating the observables.

The first example of such a observable is `await`, which takes another observable and _pauses_ the script until it observes a change to the observable. Once `await` has observed a change, the observer is disposed and the next observable is activated.

`await` takes a single observable, so if we want to wait for activity from more than one observable, we need to combine them. We already encountered `merge`, which could be used since it observes several observables and outputs the observed changes in all of them. As soon as one observable is updated, this would be propagated to `await`, causing it to continue the script.

But for this situation, the DSL also provides `any` and `all`, which propagate when one or all of the observables is updated.

```c
  script(
    await(a),
    await(b),
    await(delayed(all(a, b, c))),
    await(delayed(any(b, c))),
    await(        all(a, b, c))
  );
```

Here we need to be careful and notice the subtlety of suspended and non-suspended observables. Let's take `await(a)` first: `a` is an observable, defined outside the scope of the script. It's active and updates its value, before, during and after that `await(a)` is active. In very much the same way, `await(all(a,b,c))` must operate. When constructing the script, this will create an observable `all(a,b,c)`, which is activated at creation time. The surrounding `await` observable is initially suspended. When updates happen to `a`, `b` or `c` `all(a,b,c)` will update and track the updates until all three observables have been updated. If this happens before `await(all(a,b,c))` is activated, it may very well nog work as intended.

If we want to wait for `a`, `b` and `c`, starting at the point where `await(all(a,b,c))` is activated, we need to configure `all(a,b,c)` as such, using the `delayed` decorator. This causes `all(a,b,c)` to be marked **delayed**, therefore not processing updates until its parent (`await`) is activated.

In the script above the fifth statement (`await(all(a,b,c))`), will never introduce additional delay, because before it is activated, the awaited observable has already been updated, causing the surrounding `await` observable to dispose itself.

_To be continued..._
