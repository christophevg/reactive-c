# small script to generate 100 individual observers

# common
for i in xrange(1,101):
  print "int f%d_value; observable f%d_ob;" % (i,i)
  print "void f%d(observable o) { f%d_value = o.value; notify(f%d_ob); }" % (i,i,i)

# observer-fan
for i in xrange(1,101):
  print "add(&source, f%d);" % i

# observer-chain
for i in xrange(1,101):
  print "f%d_ob = new_observable();" % i

print "add(&source, f1);"
for i in xrange(1,100):
  print "add(&f%d_ob, f%d);" %(i,i+1)
