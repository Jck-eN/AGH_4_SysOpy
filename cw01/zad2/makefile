CC = gcc -Wall -std=c11 -ggdb

all: zad2_static tests zad2_shared tests time-tests clean

static: lib_diff.o
		ar rcs lib_diff.a lib_diff.o

shared: lib_diff.o
		$(CC) -c -fPIC lib_diff.c
		$(CC) -shared -fPIC -o lib_diff.so lib_diff.o

lib_diff.o: lib_diff.c lib_diff.h
		$(CC) -c lib_diff.c

zad2_static: main.c
		make static
		$(CC) -o main main.c lib_diff.a -I .
		make tests

zad2_shared:
		make shared
		$(CC) -o main main.c -L. lib_diff.so -I .
		make tests

clean:
		rm -f *.o

tests:
		./main create_table 10 compare_pairs small_diff1:small_diff2 small_diff1:small_diff2 remove_operation 0 2 remove_block 0

time-tests: static time_measurement.c
		make static
		$(CC) -o lab1 time_measurement.c lib_diff.a -I .

		./lab1 small_similar med_sim1:med_sim2 med_sim1:med_sim2 > raport2.txt
		./lab1 medium_similar med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 >> raport2.txt
		./lab1 big_similar med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 med_sim1:med_sim2 >> raport2.txt

		./lab1 small_medium_diff med_med1:med_med2 med_med1:med_med2 >> raport2.txt
		./lab1 medium_medium_diff med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 >> raport2.txt
		./lab1 big_medium_diff med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 med_med1:med_med2 >> raport2.txt

		./lab1 small_diff med_diff1:med_diff2 med_diff1:med_diff2 >> raport2.txt
		./lab1 medium_diff med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 >> raport2.txt
		./lab1 big_diff med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 med_diff1:med_diff2 >> raport2.txt


clean-library:
		rm -f *.so
		rm -f *.a