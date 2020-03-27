CC = gcc -Wall -ggdb

all: build tests clean clean_program

build:
	$(CC) -o main main.c

tests:
	make clean
	make gen_tests
	make copy_lib_tests
	make sort_lib_tests 
	make clean_copy
	make copy_sys_tests
	make sort_sys_tests
	make clean

gen_tests:
	./main generate t01.txt 4000 1
	./main generate t02.txt 40000 1
	./main generate t03.txt 4000 4
	./main generate t04.txt 40000 4
	./main generate t05.txt 4000 512
	./main generate t06.txt 40000 512
	./main generate t07.txt 4000 1024
	./main generate t08.txt 40000 1024
	./main generate t09.txt 4000 4096
	./main generate t10.txt 40000 4096
	./main generate t11.txt 4000 8192
	./main generate t12.txt 40000 8192

sort_lib_tests:
	./main sort t_01.txt 4000 1 lib
	./main sort t_02.txt 40000 1 lib
	./main sort t_03.txt 4000 4 lib
	./main sort t_04.txt 40000 4 lib
	./main sort t_05.txt 4000 512 lib
	./main sort t_06.txt 40000 512 lib
	./main sort t_07.txt 4000 1024 lib
	./main sort t_08.txt 40000 1024 lib
	./main sort t_09.txt 4000 4096 lib
	./main sort t_10.txt 40000 4096 lib
	./main sort t_11.txt 4000 8192 lib
	./main sort t_12.txt 40000 8192 lib

sort_sys_tests:
	./main sort t01.txt 4000 1 sys
	./main sort t02.txt 40000 1 sys
	./main sort t03.txt 4000 4 sys
	./main sort t04.txt 40000 4 sys
	./main sort t05.txt 4000 512 sys
	./main sort t06.txt 40000 512 sys
	./main sort t07.txt 4000 1024 sys
	./main sort t08.txt 40000 1024 sys
	./main sort t09.txt 4000 4096 sys
	./main sort t10.txt 40000 4096 sys
	./main sort t11.txt 4000 8192 sys
	./main sort t12.txt 40000 8192 sys

copy_sys_tests:
	./main copy t01.txt t_01.txt 4000 1 sys
	./main copy t02.txt t_02.txt 40000 1 sys
	./main copy t03.txt t_03.txt 4000 4 sys
	./main copy t04.txt t_04.txt 40000 4 sys
	./main copy t05.txt t_05.txt 4000 512 sys
	./main copy t06.txt t_06.txt 40000 512 sys
	./main copy t07.txt t_07.txt 4000 1024 sys
	./main copy t08.txt t_08.txt 40000 1024 sys
	./main copy t09.txt t_09.txt 4000 4096 sys
	./main copy t10.txt t_10.txt 40000 4096 sys
	./main copy t11.txt t_11.txt 4000 8192 sys
	./main copy t12.txt t_12.txt 40000 8192 sys

copy_lib_tests:
	./main copy t01.txt t_01.txt 4000 1 lib
	./main copy t02.txt t_02.txt 40000 1 lib
	./main copy t03.txt t_03.txt 4000 4 lib
	./main copy t04.txt t_04.txt 40000 4 lib
	./main copy t05.txt t_05.txt 4000 512 lib
	./main copy t06.txt t_06.txt 40000 512 lib
	./main copy t07.txt t_07.txt 4000 1024 lib
	./main copy t08.txt t_08.txt 40000 1024 lib
	./main copy t09.txt t_09.txt 4000 4096 lib
	./main copy t10.txt t_10.txt 40000 4096 lib
	./main copy t11.txt t_11.txt 4000 8192 lib
	./main copy t12.txt t_12.txt 40000 8192 lib


clean:
	rm -f t*.txt

clean_copy:
	rm -f t_*.txt

clean_program:
	rm -f main