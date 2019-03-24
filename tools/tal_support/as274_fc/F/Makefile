.SUFFIXES: .f .test
.f.test :
	$(F77) -o $@ $(FFLAGS) $< wichman-hill.o as274.o

TESTS = t1.test t2.test t3.test t4.test t5.test t6.test t7.test \
	t8.test t9.test t10.test

all : as274.o wichman-hill.o $(TESTS)

reg : all
	-t1.test | diff f1.expect -
	-t2.test | diff f2.expect -
	-t3.test | diff f3.expect -
	-t4.test | diff f4.expect -
	-t5.test | diff f5.expect -
	-t6.test | diff f6.expect -
	-t7.test | diff f7.expect -
	-t8.test | diff f8.expect -
	-t9.test | diff f9.expect -
	-test.t10 | diff f10.expect -

recreate : all
	t1.test > f1.expect
	t2.test > f2.expect
	t3.test > f3.expect
	t4.test > f4.expect
	t5.test > f5.expect
	t6.test > f6.expect
	t7.test > f7.expect
	t8.test > f8.expect
	t9.test > f9.expect
	test.t10 > f10.expect

clean : 
	rm -f *.o a.out $(TESTS) core

