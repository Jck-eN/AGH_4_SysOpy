CC = gcc -Wall -ggdb

all: build test clean

build: main.c helper.h pack2.c receive1.c send3.c
	${CC} -o main main.c
	${CC} -o receive1 receive1.c
	${CC} -o pack2 pack2.c
	${CC} -o send3 send3.c

test: main
	./main

clean:
	rm main receive1 pack2 send3 