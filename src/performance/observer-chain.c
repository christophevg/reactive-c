#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/time.h"

// a minimalistic observer pattern implementation

typedef struct observable observable;

typedef void (*observing_function)(observable);

typedef struct observer {
  observing_function f;
  struct observer *next;
} observer;

struct observable {
  int      value;
  observer *observer;
};

observable new_observable() {
  observable o;
  o.value = 0;
  o.observer = NULL;
  return o;
}

void add(observable *o, observing_function f) {
  observer *n = malloc(sizeof(struct observer));
  n->next = o->observer;
  n->f = f;
  o->observer = n;
}

// timed range

void notify(observable o) {
  int c = 0;
  observer *i = o.observer;
  while(i) {
    c++;
    i->f(o);
    i = i->next;
  }
}

void update(observable o, int value) {
  o.value = value;
  notify(o);
}

#define FAN_WIDTH        100
#define UPDATES      5000000
#define ITERATIONS         5

int f1_value; observable f1_ob;
void f1(observable o) { f1_value = o.value; notify(f1_ob); }
int f2_value; observable f2_ob;
void f2(observable o) { f2_value = o.value; notify(f2_ob); }
int f3_value; observable f3_ob;
void f3(observable o) { f3_value = o.value; notify(f3_ob); }
int f4_value; observable f4_ob;
void f4(observable o) { f4_value = o.value; notify(f4_ob); }
int f5_value; observable f5_ob;
void f5(observable o) { f5_value = o.value; notify(f5_ob); }
int f6_value; observable f6_ob;
void f6(observable o) { f6_value = o.value; notify(f6_ob); }
int f7_value; observable f7_ob;
void f7(observable o) { f7_value = o.value; notify(f7_ob); }
int f8_value; observable f8_ob;
void f8(observable o) { f8_value = o.value; notify(f8_ob); }
int f9_value; observable f9_ob;
void f9(observable o) { f9_value = o.value; notify(f9_ob); }
int f10_value; observable f10_ob;
void f10(observable o) { f10_value = o.value; notify(f10_ob); }
int f11_value; observable f11_ob;
void f11(observable o) { f11_value = o.value; notify(f11_ob); }
int f12_value; observable f12_ob;
void f12(observable o) { f12_value = o.value; notify(f12_ob); }
int f13_value; observable f13_ob;
void f13(observable o) { f13_value = o.value; notify(f13_ob); }
int f14_value; observable f14_ob;
void f14(observable o) { f14_value = o.value; notify(f14_ob); }
int f15_value; observable f15_ob;
void f15(observable o) { f15_value = o.value; notify(f15_ob); }
int f16_value; observable f16_ob;
void f16(observable o) { f16_value = o.value; notify(f16_ob); }
int f17_value; observable f17_ob;
void f17(observable o) { f17_value = o.value; notify(f17_ob); }
int f18_value; observable f18_ob;
void f18(observable o) { f18_value = o.value; notify(f18_ob); }
int f19_value; observable f19_ob;
void f19(observable o) { f19_value = o.value; notify(f19_ob); }
int f20_value; observable f20_ob;
void f20(observable o) { f20_value = o.value; notify(f20_ob); }
int f21_value; observable f21_ob;
void f21(observable o) { f21_value = o.value; notify(f21_ob); }
int f22_value; observable f22_ob;
void f22(observable o) { f22_value = o.value; notify(f22_ob); }
int f23_value; observable f23_ob;
void f23(observable o) { f23_value = o.value; notify(f23_ob); }
int f24_value; observable f24_ob;
void f24(observable o) { f24_value = o.value; notify(f24_ob); }
int f25_value; observable f25_ob;
void f25(observable o) { f25_value = o.value; notify(f25_ob); }
int f26_value; observable f26_ob;
void f26(observable o) { f26_value = o.value; notify(f26_ob); }
int f27_value; observable f27_ob;
void f27(observable o) { f27_value = o.value; notify(f27_ob); }
int f28_value; observable f28_ob;
void f28(observable o) { f28_value = o.value; notify(f28_ob); }
int f29_value; observable f29_ob;
void f29(observable o) { f29_value = o.value; notify(f29_ob); }
int f30_value; observable f30_ob;
void f30(observable o) { f30_value = o.value; notify(f30_ob); }
int f31_value; observable f31_ob;
void f31(observable o) { f31_value = o.value; notify(f31_ob); }
int f32_value; observable f32_ob;
void f32(observable o) { f32_value = o.value; notify(f32_ob); }
int f33_value; observable f33_ob;
void f33(observable o) { f33_value = o.value; notify(f33_ob); }
int f34_value; observable f34_ob;
void f34(observable o) { f34_value = o.value; notify(f34_ob); }
int f35_value; observable f35_ob;
void f35(observable o) { f35_value = o.value; notify(f35_ob); }
int f36_value; observable f36_ob;
void f36(observable o) { f36_value = o.value; notify(f36_ob); }
int f37_value; observable f37_ob;
void f37(observable o) { f37_value = o.value; notify(f37_ob); }
int f38_value; observable f38_ob;
void f38(observable o) { f38_value = o.value; notify(f38_ob); }
int f39_value; observable f39_ob;
void f39(observable o) { f39_value = o.value; notify(f39_ob); }
int f40_value; observable f40_ob;
void f40(observable o) { f40_value = o.value; notify(f40_ob); }
int f41_value; observable f41_ob;
void f41(observable o) { f41_value = o.value; notify(f41_ob); }
int f42_value; observable f42_ob;
void f42(observable o) { f42_value = o.value; notify(f42_ob); }
int f43_value; observable f43_ob;
void f43(observable o) { f43_value = o.value; notify(f43_ob); }
int f44_value; observable f44_ob;
void f44(observable o) { f44_value = o.value; notify(f44_ob); }
int f45_value; observable f45_ob;
void f45(observable o) { f45_value = o.value; notify(f45_ob); }
int f46_value; observable f46_ob;
void f46(observable o) { f46_value = o.value; notify(f46_ob); }
int f47_value; observable f47_ob;
void f47(observable o) { f47_value = o.value; notify(f47_ob); }
int f48_value; observable f48_ob;
void f48(observable o) { f48_value = o.value; notify(f48_ob); }
int f49_value; observable f49_ob;
void f49(observable o) { f49_value = o.value; notify(f49_ob); }
int f50_value; observable f50_ob;
void f50(observable o) { f50_value = o.value; notify(f50_ob); }
int f51_value; observable f51_ob;
void f51(observable o) { f51_value = o.value; notify(f51_ob); }
int f52_value; observable f52_ob;
void f52(observable o) { f52_value = o.value; notify(f52_ob); }
int f53_value; observable f53_ob;
void f53(observable o) { f53_value = o.value; notify(f53_ob); }
int f54_value; observable f54_ob;
void f54(observable o) { f54_value = o.value; notify(f54_ob); }
int f55_value; observable f55_ob;
void f55(observable o) { f55_value = o.value; notify(f55_ob); }
int f56_value; observable f56_ob;
void f56(observable o) { f56_value = o.value; notify(f56_ob); }
int f57_value; observable f57_ob;
void f57(observable o) { f57_value = o.value; notify(f57_ob); }
int f58_value; observable f58_ob;
void f58(observable o) { f58_value = o.value; notify(f58_ob); }
int f59_value; observable f59_ob;
void f59(observable o) { f59_value = o.value; notify(f59_ob); }
int f60_value; observable f60_ob;
void f60(observable o) { f60_value = o.value; notify(f60_ob); }
int f61_value; observable f61_ob;
void f61(observable o) { f61_value = o.value; notify(f61_ob); }
int f62_value; observable f62_ob;
void f62(observable o) { f62_value = o.value; notify(f62_ob); }
int f63_value; observable f63_ob;
void f63(observable o) { f63_value = o.value; notify(f63_ob); }
int f64_value; observable f64_ob;
void f64(observable o) { f64_value = o.value; notify(f64_ob); }
int f65_value; observable f65_ob;
void f65(observable o) { f65_value = o.value; notify(f65_ob); }
int f66_value; observable f66_ob;
void f66(observable o) { f66_value = o.value; notify(f66_ob); }
int f67_value; observable f67_ob;
void f67(observable o) { f67_value = o.value; notify(f67_ob); }
int f68_value; observable f68_ob;
void f68(observable o) { f68_value = o.value; notify(f68_ob); }
int f69_value; observable f69_ob;
void f69(observable o) { f69_value = o.value; notify(f69_ob); }
int f70_value; observable f70_ob;
void f70(observable o) { f70_value = o.value; notify(f70_ob); }
int f71_value; observable f71_ob;
void f71(observable o) { f71_value = o.value; notify(f71_ob); }
int f72_value; observable f72_ob;
void f72(observable o) { f72_value = o.value; notify(f72_ob); }
int f73_value; observable f73_ob;
void f73(observable o) { f73_value = o.value; notify(f73_ob); }
int f74_value; observable f74_ob;
void f74(observable o) { f74_value = o.value; notify(f74_ob); }
int f75_value; observable f75_ob;
void f75(observable o) { f75_value = o.value; notify(f75_ob); }
int f76_value; observable f76_ob;
void f76(observable o) { f76_value = o.value; notify(f76_ob); }
int f77_value; observable f77_ob;
void f77(observable o) { f77_value = o.value; notify(f77_ob); }
int f78_value; observable f78_ob;
void f78(observable o) { f78_value = o.value; notify(f78_ob); }
int f79_value; observable f79_ob;
void f79(observable o) { f79_value = o.value; notify(f79_ob); }
int f80_value; observable f80_ob;
void f80(observable o) { f80_value = o.value; notify(f80_ob); }
int f81_value; observable f81_ob;
void f81(observable o) { f81_value = o.value; notify(f81_ob); }
int f82_value; observable f82_ob;
void f82(observable o) { f82_value = o.value; notify(f82_ob); }
int f83_value; observable f83_ob;
void f83(observable o) { f83_value = o.value; notify(f83_ob); }
int f84_value; observable f84_ob;
void f84(observable o) { f84_value = o.value; notify(f84_ob); }
int f85_value; observable f85_ob;
void f85(observable o) { f85_value = o.value; notify(f85_ob); }
int f86_value; observable f86_ob;
void f86(observable o) { f86_value = o.value; notify(f86_ob); }
int f87_value; observable f87_ob;
void f87(observable o) { f87_value = o.value; notify(f87_ob); }
int f88_value; observable f88_ob;
void f88(observable o) { f88_value = o.value; notify(f88_ob); }
int f89_value; observable f89_ob;
void f89(observable o) { f89_value = o.value; notify(f89_ob); }
int f90_value; observable f90_ob;
void f90(observable o) { f90_value = o.value; notify(f90_ob); }
int f91_value; observable f91_ob;
void f91(observable o) { f91_value = o.value; notify(f91_ob); }
int f92_value; observable f92_ob;
void f92(observable o) { f92_value = o.value; notify(f92_ob); }
int f93_value; observable f93_ob;
void f93(observable o) { f93_value = o.value; notify(f93_ob); }
int f94_value; observable f94_ob;
void f94(observable o) { f94_value = o.value; notify(f94_ob); }
int f95_value; observable f95_ob;
void f95(observable o) { f95_value = o.value; notify(f95_ob); }
int f96_value; observable f96_ob;
void f96(observable o) { f96_value = o.value; notify(f96_ob); }
int f97_value; observable f97_ob;
void f97(observable o) { f97_value = o.value; notify(f97_ob); }
int f98_value; observable f98_ob;
void f98(observable o) { f98_value = o.value; notify(f98_ob); }
int f99_value; observable f99_ob;
void f99(observable o) { f99_value = o.value; notify(f99_ob); }
int f100_value; observable f100_ob;
void f100(observable o) { f100_value = o.value; notify(f100_ob); }

int main(void) {
  // source
  observable source = new_observable();
  
  f1_ob = new_observable();
  f2_ob = new_observable();
  f3_ob = new_observable();
  f4_ob = new_observable();
  f5_ob = new_observable();
  f6_ob = new_observable();
  f7_ob = new_observable();
  f8_ob = new_observable();
  f9_ob = new_observable();
  f10_ob = new_observable();
  f11_ob = new_observable();
  f12_ob = new_observable();
  f13_ob = new_observable();
  f14_ob = new_observable();
  f15_ob = new_observable();
  f16_ob = new_observable();
  f17_ob = new_observable();
  f18_ob = new_observable();
  f19_ob = new_observable();
  f20_ob = new_observable();
  f21_ob = new_observable();
  f22_ob = new_observable();
  f23_ob = new_observable();
  f24_ob = new_observable();
  f25_ob = new_observable();
  f26_ob = new_observable();
  f27_ob = new_observable();
  f28_ob = new_observable();
  f29_ob = new_observable();
  f30_ob = new_observable();
  f31_ob = new_observable();
  f32_ob = new_observable();
  f33_ob = new_observable();
  f34_ob = new_observable();
  f35_ob = new_observable();
  f36_ob = new_observable();
  f37_ob = new_observable();
  f38_ob = new_observable();
  f39_ob = new_observable();
  f40_ob = new_observable();
  f41_ob = new_observable();
  f42_ob = new_observable();
  f43_ob = new_observable();
  f44_ob = new_observable();
  f45_ob = new_observable();
  f46_ob = new_observable();
  f47_ob = new_observable();
  f48_ob = new_observable();
  f49_ob = new_observable();
  f50_ob = new_observable();
  f51_ob = new_observable();
  f52_ob = new_observable();
  f53_ob = new_observable();
  f54_ob = new_observable();
  f55_ob = new_observable();
  f56_ob = new_observable();
  f57_ob = new_observable();
  f58_ob = new_observable();
  f59_ob = new_observable();
  f60_ob = new_observable();
  f61_ob = new_observable();
  f62_ob = new_observable();
  f63_ob = new_observable();
  f64_ob = new_observable();
  f65_ob = new_observable();
  f66_ob = new_observable();
  f67_ob = new_observable();
  f68_ob = new_observable();
  f69_ob = new_observable();
  f70_ob = new_observable();
  f71_ob = new_observable();
  f72_ob = new_observable();
  f73_ob = new_observable();
  f74_ob = new_observable();
  f75_ob = new_observable();
  f76_ob = new_observable();
  f77_ob = new_observable();
  f78_ob = new_observable();
  f79_ob = new_observable();
  f80_ob = new_observable();
  f81_ob = new_observable();
  f82_ob = new_observable();
  f83_ob = new_observable();
  f84_ob = new_observable();
  f85_ob = new_observable();
  f86_ob = new_observable();
  f87_ob = new_observable();
  f88_ob = new_observable();
  f89_ob = new_observable();
  f90_ob = new_observable();
  f91_ob = new_observable();
  f92_ob = new_observable();
  f93_ob = new_observable();
  f94_ob = new_observable();
  f95_ob = new_observable();
  f96_ob = new_observable();
  f97_ob = new_observable();
  f98_ob = new_observable();
  f99_ob = new_observable();
  f100_ob = new_observable();

  add(&source, f1);
  add(&f1_ob, f2);
  add(&f2_ob, f3);
  add(&f3_ob, f4);
  add(&f4_ob, f5);
  add(&f5_ob, f6);
  add(&f6_ob, f7);
  add(&f7_ob, f8);
  add(&f8_ob, f9);
  add(&f9_ob, f10);
  add(&f10_ob, f11);
  add(&f11_ob, f12);
  add(&f12_ob, f13);
  add(&f13_ob, f14);
  add(&f14_ob, f15);
  add(&f15_ob, f16);
  add(&f16_ob, f17);
  add(&f17_ob, f18);
  add(&f18_ob, f19);
  add(&f19_ob, f20);
  add(&f20_ob, f21);
  add(&f21_ob, f22);
  add(&f22_ob, f23);
  add(&f23_ob, f24);
  add(&f24_ob, f25);
  add(&f25_ob, f26);
  add(&f26_ob, f27);
  add(&f27_ob, f28);
  add(&f28_ob, f29);
  add(&f29_ob, f30);
  add(&f30_ob, f31);
  add(&f31_ob, f32);
  add(&f32_ob, f33);
  add(&f33_ob, f34);
  add(&f34_ob, f35);
  add(&f35_ob, f36);
  add(&f36_ob, f37);
  add(&f37_ob, f38);
  add(&f38_ob, f39);
  add(&f39_ob, f40);
  add(&f40_ob, f41);
  add(&f41_ob, f42);
  add(&f42_ob, f43);
  add(&f43_ob, f44);
  add(&f44_ob, f45);
  add(&f45_ob, f46);
  add(&f46_ob, f47);
  add(&f47_ob, f48);
  add(&f48_ob, f49);
  add(&f49_ob, f50);
  add(&f50_ob, f51);
  add(&f51_ob, f52);
  add(&f52_ob, f53);
  add(&f53_ob, f54);
  add(&f54_ob, f55);
  add(&f55_ob, f56);
  add(&f56_ob, f57);
  add(&f57_ob, f58);
  add(&f58_ob, f59);
  add(&f59_ob, f60);
  add(&f60_ob, f61);
  add(&f61_ob, f62);
  add(&f62_ob, f63);
  add(&f63_ob, f64);
  add(&f64_ob, f65);
  add(&f65_ob, f66);
  add(&f66_ob, f67);
  add(&f67_ob, f68);
  add(&f68_ob, f69);
  add(&f69_ob, f70);
  add(&f70_ob, f71);
  add(&f71_ob, f72);
  add(&f72_ob, f73);
  add(&f73_ob, f74);
  add(&f74_ob, f75);
  add(&f75_ob, f76);
  add(&f76_ob, f77);
  add(&f77_ob, f78);
  add(&f78_ob, f79);
  add(&f79_ob, f80);
  add(&f80_ob, f81);
  add(&f81_ob, f82);
  add(&f82_ob, f83);
  add(&f83_ob, f84);
  add(&f84_ob, f85);
  add(&f85_ob, f86);
  add(&f86_ob, f87);
  add(&f87_ob, f88);
  add(&f88_ob, f89);
  add(&f89_ob, f90);
  add(&f90_ob, f91);
  add(&f91_ob, f92);
  add(&f92_ob, f93);
  add(&f93_ob, f94);
  add(&f94_ob, f95);
  add(&f95_ob, f96);
  add(&f96_ob, f97);
  add(&f97_ob, f98);
  add(&f98_ob, f99);
  add(&f99_ob, f100);

  int total = 0;

  for(int iteration=0; iteration<ITERATIONS; iteration++) {
    struct timespec start = now();
  
    for(int i=0; i<UPDATES; i++) {
      update(source, i);
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
