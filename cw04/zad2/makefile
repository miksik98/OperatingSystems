CC = gcc
FLAGS = -Wall -O2

tests:  compile
	mkdir archiwum
	touch lista
	echo -e "test1 1\ntest2 2\ntest3 3\n" > lista
	touch test1 test2 test3
	./tester test1 1 7 30
	./tester test2 1 5 15
	./tester test3 1 3 10
	./monitor lista 15
	./monitor lista 15 

compile:
	$(CC) $(FLAGS) -o monitor monitor.c
	$(CC) $(FLAGS) -o tester tester.c

clean:
	rm lista test1 test2 test3
	rm -rf archiwum monitor tester
