FLAGS = gcc -Wall -std=c99

static:
	$(FLAGS) -c library.c
	ar rcs library.a library.o

shared:
	$(FLAGS) -c -fPIC library.c
	$(FLAGS) -shared -fPIC -o library.so library.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
