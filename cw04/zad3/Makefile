CC = gcc -Wall -ggdb

all: clean build test

build: main.c
	${CC} -o main main.c

test: 
	./main child >> result3.txt
	./main queue >> result3.txt
	./main int >> result3.txt

clean:
	rm main
	rm result3.txt