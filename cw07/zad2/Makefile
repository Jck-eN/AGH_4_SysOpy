CC = gcc -Wall -ggdb

all: build test clean

build: main.c helper.h pack2.c receive1.c send3.c
	${CC} -o main main.c -lrt -lpthread
	${CC} -o receive1 receive1.c -lrt -lpthread
	${CC} -o pack2 pack2.c -lrt -lpthread
	${CC} -o send3 send3.c -lrt -lpthread

test: main
	./main

clean:
	rm main receive1 pack2 send3 