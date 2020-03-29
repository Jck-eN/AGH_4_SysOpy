CC = gcc -Wall -ggdb

all: clean build test

build: main.c second.c
	${CC} -o main main.c
	${CC} -o second second.c

test: 
	echo "ignore child:\n" >> report2.txt
	./main ignore child >> report2.txt
	sleep 1
	echo "handler child:\n" >> report2.txt
	./main handler child >> report2.txt
	sleep 1
	echo "mask child:\n" >> report2.txt
	./main mask child >> report2.txt
	sleep 1
	echo "pending child:\n" >> report2.txt
	./main pending child >> report2.txt
	sleep 1
	echo "\n\nignore exec:\n" >> report2.txt
	./main ignore exec >> report2.txt
	sleep 1
	echo "mask exec:\n" >> report2.txt
	./main mask exec >> report2.txt
	sleep 1
	echo "pending exec:\n" >> report2.txt
	./main pending exec >> report2.txt

clean:
	rm -f main
	rm -f report2.txt