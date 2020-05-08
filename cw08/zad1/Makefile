CC = gcc -Wall -ggdb

all: build test

build: main.c 
	${CC} -o main main.c -lpthread

test: main
	for method in sign block interleaved ; do \
		for thread_num in 1 2 4 8 16; do \
			printf "$$thread_num wątków - metoda  $$method\n" >> results.txt ; \
			rm -rf ./out_$$method ; \
			./main $$thread_num $$method ./img/image.ascii.pgm ./out_$$method.ascii.pgm >> results.txt ; \
			printf "\n\n" >> results.txt ; \
		done ; \
		printf "\n--------------------\n" >> results.txt ; \
	done ;\
	printf "\nDone!\n"

clean:
	rm ./main 
	rm ./results.txt
	rm ./out_*

