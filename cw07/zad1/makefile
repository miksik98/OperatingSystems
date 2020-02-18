CC = gcc -Wall

all: clean main

main:
	$(CC) -o trucker trucker.c conveyor_belt.c
	$(CC) -o loader loader.c conveyor_belt.c


clean:
	rm -f trucker loader
