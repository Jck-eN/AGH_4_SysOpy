CC = gcc -Wall -std=c11 -ggdb
TASK = 3a

all: static time-tests clean

static: lib_diff.c time_measurement.c
		$(CC) -c lib_diff.c
		ar rcs lib_diff.a lib_diff.o
		$(CC) -o lab1 time_measurement.c lib_diff.a

shared: lib_diff.o
		$(CC) -c -fPIC lib_diff.c
		$(CC) -shared -fPIC -o lib_diff.so lib_diff.o
		$(CC) -o lab1 time_measurement.c lib_diff.so -L. -Wl,-rpath=`pwd`
dynamic: time_measurement_dyn.c shared
		$(CC) -o lab1 time_measurement_dyn.c -L. -ldl -D DYNAMIC

lib_diff.o: lib_diff.c lib_diff.h
		$(CC) -c lib_diff.c

clean:
		rm -f *.o

time-tests: static time_measurement.c
		./lab1 small_similar med_sim1:med_sim2 med_sim1:med_sim2 >> raport$(TASK).txt
		./lab1 medium_similar med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 >> raport$(TASK).txt
		./lab1 big_similar med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 >> raport$(TASK).txt

		./lab1 small_medium_diff med_med1:med_med2 med_med1:med_med2 >> raport$(TASK).txt
		./lab1 medium_medium_diff med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 >> raport$(TASK).txt
		./lab1 big_medium_diff med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 >> raport$(TASK).txt

		./lab1 small_diff med_diff1:med_diff2 med_diff1:med_diff2 >> raport$(TASK).txt
		./lab1 medium_diff med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 >> raport$(TASK).txt
		./lab1 big_diff med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 >> raport$(TASK).txt

time-tests-static: static time-tests
time-tests-shared: shared time-tests



clean-library:
		rm -f *.so
		rm -f *.a
