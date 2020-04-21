CC = gcc -Wall -ggdb

all: test

build: client.c server.c helper.h
	${CC} -o server server.c
	${CC} -o client client.c

test: server client
	echo 'Please, run programs manually to use chat system'

clean:
	rm server client