CC = gcc
FLAGS = -Wall -std=c99
LIB_NAME = library
MAIN_SRC = main
FILE = results3a.txt
DIR1 = /home
DIR2 = /home/mikolajsikora
DIR3 = /home/mikolajsikora/test2

compile: static shared dynamic

static:
	$(CC) $(FLAGS) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o
	$(CC) $(FLAGS) $(MAIN_SRC).c -I. $(LIB_NAME).a -o $(MAIN_SRC)_static
	make clean

shared:
	export LD_LIBRARY_PATH=$LD_LIBARY_PATH:.
	$(CC) $(FLAGS) -c -fPIC $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) $(FLAGS) -shared -fPIC -o $(LIB_NAME).so $(LIB_NAME).o
	$(CC) $(FLAGS) -L. $(LIB_NAME).so -o $(MAIN_SRC)_shared $(MAIN_SRC).c
		

dynamic:
	make shared
	$(CC) $(FLAGS) -o $(MAIN_SRC)_dyn $(MAIN_SRC)_dyn.c -ldl
	make clean

clean:
	rm -f *.o
	rm -f *.a

clean_all: clean
	rm -f *.so
	rm -f $(MAIN_SRC)_dyn
	rm -f $(MAIN_SRC)_shared
	rm -f $(MAIN_SRC)_static

test: compile
	echo 'static library' >> $(FILE)
	./$(MAIN_SRC)_static test_searching $(DIR1) $(DIR2) $(DIR3) "*a*" | tee -a $(FILE)
	./$(MAIN_SRC)_static test_add_remove "a.txt" "b.txt" "c.txt" | tee -a $(FILE)
	./$(MAIN_SRC)_static test_ar_loop 100 "a.txt" | tee -a $(FILE)
	echo 'shared library' >> $(FILE)
	./$(MAIN_SRC)_shared test_searching $(DIR1) $(DIR2) $(DIR3) "*a*" | tee -a $(FILE)
	./$(MAIN_SRC)_shared test_add_remove "a.txt" "b.txt" "c.txt" | tee -a $(FILE)
	./$(MAIN_SRC)_shared test_ar_loop 100 "a.txt" | tee -a $(FILE)
	echo 'dynamic library' >> $(FILE)
	./$(MAIN_SRC)_dyn test_searching $(DIR1) $(DIR2) $(DIR3) "*a*" | tee -a $(FILE)
	./$(MAIN_SRC)_dyn test_add_remove "a.txt" "b.txt" "c.txt" | tee -a $(FILE)
	./$(MAIN_SRC)_dyn test_ar_loop 100 "a.txt" | tee -a $(FILE)
	make clean_all
