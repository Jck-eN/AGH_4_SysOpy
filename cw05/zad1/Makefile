CC = gcc -Wall -ggdb

all: test

build: main.c
	${CC} -o main main.c

test: main
	./main commands 

clean:
	rm main