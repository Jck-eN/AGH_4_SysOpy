CC = gcc -Wall -ggdb

all: test

build: main.c
	${CC} -o main main.c

test: main
	./main unsorted 

clean:
	rm main