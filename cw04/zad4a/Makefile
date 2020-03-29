CC = gcc -Wall -ggdb

all: build

build: sender.c catcher.c
	${CC} -o sender sender.c
	${CC} -o catcher catcher.c

test: 
	echo "Run programs manually to test it - required process id"

clean:
	rm sender
	rm catcher