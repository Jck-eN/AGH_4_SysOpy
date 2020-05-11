CC = gcc -Wall -ggdb

all: build test

build: main.c 
	${CC} -o main main.c -lpthread

test: main
	./main 3 12

clean:
	rm ./main 

