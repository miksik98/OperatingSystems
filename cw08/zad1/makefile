CC = gcc
MAIN_SRC = main
FLAGS = -Wall
INPUT = MonaLisa.pgm
OUTPUT = Result.pgm
MODE1 = block
MODE2 = interleaved
FILTER1 = filter1
FILTER2 = filter2

compile:    clean
	$(CC) $(FLAGS) -o main $(MAIN_SRC).c -lpthread -lm

test:   compile
	touch Times.txt
	echo -e "block test" >> Times.txt
	echo -e "\nfilter (5x5)" >> Times.txt
	echo -e "\n1 thread" > Times.txt
	./$(MAIN_SRC) 1 $(MODE1) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n2 threads" >> Times.txt
	./$(MAIN_SRC) 2 $(MODE1) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n4 threads" >> Times.txt
	./$(MAIN_SRC) 4 $(MODE1) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n8 threads" >> Times.txt
	./$(MAIN_SRC) 8 $(MODE1) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	rm $(OUTPUT)
	echo -e "\n-----------------------------" >> Times.txt
	echo -e "\nfilter (20x20)" >> Times.txt
	echo -e "\n1 thread" >> Times.txt
	./$(MAIN_SRC) 1 $(MODE1) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n2 threads" >> Times.txt
	./$(MAIN_SRC) 2 $(MODE1) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n4 threads" >> Times.txt
	./$(MAIN_SRC) 4 $(MODE1) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n8 threads" >> Times.txt
	./$(MAIN_SRC) 8 $(MODE1) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	rm $(OUTPUT)
	echo -e "\n-----------------------------" >> Times.txt
	echo "-----------------------------" >> Times.txt
	echo -e "intervaled test" >> Times.txt
	echo -e "\nfilter (5x5)" >> Times.txt
	echo -e "\n1 thread" >> Times.txt
	./$(MAIN_SRC) 1 $(MODE2) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n2 threads" >> Times.txt
	./$(MAIN_SRC) 2 $(MODE2) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n4 threads" >> Times.txt
	./$(MAIN_SRC) 4 $(MODE2) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	echo -e "\n8 threads" >> Times.txt
	./$(MAIN_SRC) 8 $(MODE2) $(INPUT) $(FILTER1) $(OUTPUT) >> Times.txt
	rm $(OUTPUT)
	echo -e "\n-----------------------------" >> Times.txt
	echo -e "\nfilter2 (20x20)" >> Times.txt
	echo -e "\n1 thread" >> Times.txt
	./$(MAIN_SRC) 1 $(MODE2) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n2 threads" >> Times.txt
	./$(MAIN_SRC) 2 $(MODE2) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n4 threads" >> Times.txt
	./$(MAIN_SRC) 4 $(MODE2) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt
	echo -e "\n8 threads" >> Times.txt
	./$(MAIN_SRC) 8 $(MODE2) $(INPUT) $(FILTER2) $(OUTPUT) >> Times.txt

clean:
	rm -f main
