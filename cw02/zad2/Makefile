CC = gcc -Wall -ggdb

all: 
	make build
	make tests
	make clean

build:
	$(CC) -o main main.c

tests:
	mkdir test1
	mkdir test2
	touch test2/file1
	touch test1/file1
	echo "test" > test1/file1
	touch test1/file2
	ln -s test2 test1
	./main . -atime -2 -maxdepth 5 > result.txt


clean:
	rm -f main
	rm -rf test1
	rm -rf test2
