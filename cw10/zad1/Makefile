CC = gcc -Wall -ggdb

all: build test

build: client.c server.c common.h
	${CC} -o server server.c -lpthread
	${CC} -o client client.c -lpthread

test: server client
	echo 'Please, run programs manually to play tic-tac-toe'
	echo './server 2345 /tmp/socket'
	echo './client player1 local /tmp/socket'
	echo './client player2 remote 127.0.0.1:2345'

clean:
	rm server client