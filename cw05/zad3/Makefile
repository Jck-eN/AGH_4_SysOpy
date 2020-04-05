CC = gcc -Wall -ggdb

all: build test clean

build: main.c producer.c consumer.c
	mkfifo pipe
	${CC} -o main main.c
	${CC} -o producer producer.c
	${CC} -o consumer consumer.c

test: main producer consumer
	./main 

clean:
	rm main
	rm producer
	rm consumer
	rm pipe