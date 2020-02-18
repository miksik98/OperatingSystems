CC = gcc
CFLAGS = -Wall -pthread
MAIN = main

compile:
	$(CC) $(CFLAGS) $(MAIN).c -o $(MAIN)

test:	compile
	./$(MAIN) 40 4 10 10

clean:	
	rm -r main
