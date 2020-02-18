CC = gcc
CFLAGS = -Wall

all: server client	

chat: chat.h chat.c

server: server.c chat.c chat.h
	$(CC) $(CFLAGS) server.c chat.c -o server -lrt 

client: client.c chat.c chat.h
	$(CC) $(CFLAGS) client.c chat.c -o client -lrt

clean:	
	rm -r server client
