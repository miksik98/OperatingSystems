CC = gcc
FLAGS = -Wall
MAIN_SRC = main

test: 	compile
	./wynik test.txt
	make clean

compile:
	$(CC) $(FLAGS) -o wynik $(MAIN_SRC).c

clean:
	rm -f wynik
