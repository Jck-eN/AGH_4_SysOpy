CC = gcc -Wall -ggdb

all: 
	make clean
	make build
	make generate_sample
	make calc_tmp
	echo '\ncalc_tmp:\n' > results.txt
	make test
	make generate_sample2
	make calc_flock
	echo '\ncalc_flock:\n' >> results.txt
	make test
	make clean

build:
	$(CC) -o macierz macierz.c
	$(CC) -o generate generate_matrices.c
	$(CC) -o tester tester.c

generate_sample: 
	./generate 3 300 310

generate_sample2:
	./generate 3 300 310

calc_tmp:
	./macierz lista 3 10 sep 2 100

calc_flock:
	./macierz lista 3 10 com 2 100

test:
	./tester lista >> results.txt



clean:
	rm -f macierz
	rm -f generate
	rm -f tester
	rm -f tmp*
	rm -f lista
	rm -f m_*
	rm -f res_*