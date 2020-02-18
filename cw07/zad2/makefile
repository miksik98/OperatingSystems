CC = gcc -Wall

all: clean main

main:
	$(CC) -o trucker trucker.c conveyor_belt.c -lpthread -lrt
	$(CC) -o loader loader.c conveyor_belt.c -lpthread -lrt


clean:
	rm -f trucker loader
