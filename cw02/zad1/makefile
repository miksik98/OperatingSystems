CC = gcc
FLAGS = -Wall -std=c99
MAIN_SRC = main
FILE = wyniki1.txt
FILE1 = plik1
FILE2 = plik2
DEST1 = plik3
DEST2 = plik4

compile:
	$(CC) $(FLAGS) -o wynik $(MAIN_SRC).c

clean:
	rm -f *.o
	rm -f $(FILE1)
	rm -f $(FILE2)
	rm -f $(DEST1)
	rm -f $(DEST2)

clean_all: 
	rm -f $(FILE)
	rm -f wynik
	make clean

test:	compile
	for n in 1 4 512 1024 4096 8196; do \
		for s in 3000 5000; do\
			echo "TESTS FOR SIZE = $$n, nr of records: $$s" >> $(FILE); \
			./wynik generate $(FILE1) $$s $$n; \
			./wynik test copy $(FILE1) $(DEST1) $$s $$n sys | tee -a $(FILE); \
			./wynik test copy $(FILE1) $(DEST2) $$s $$n lib | tee -a $(FILE); \
			./wynik test sort $(DEST1) $$s $$n sys | tee -a $(FILE); \
			./wynik test sort $(DEST2) $$s $$n lib | tee -a $(FILE); \
			make clean; \
		done; \
	done
	#echo "TESTS FOR SIZE = 8192" >> $(FILE)
	#./wynik generate $(FILE1) 8192 250
	#./wynik generate $(FILE2) 8192 500
	#./wynik test copy $(FILE1) $(DEST1) 8192 250 sys | tee -a $(FILE)
	#./wynik test copy $(FILE1) $(DEST1) 8192 250 lib | tee -a $(FILE)
	#./wynik test copy $(FILE2) $(DEST2) 8192 500 sys | tee -a $(FILE)
	#./wynik test copy $(FILE2) $(DEST2) 8192 500 lib | tee -a $(FILE)
	#./wynik test sort $(FILE1) 8192 250 sys | tee -a $(FILE)
	#./wynik test sort $(DEST1) 8192 250 lib | tee -a $(FILE)
	#./wynik test sort $(FILE2) 8192 500 sys | tee -a $(FILE)
	#./wynik test sort $(DEST2) 8192 500 lib | tee -a $(FILE)
	#make clean

