all: main.o utils.o
	gcc -Wall main.o utils.o -lrt -lpthread -o main
	@echo "\n> Run with : ./main -i filepath"

main.o: main.c
	gcc -c -Wall -pedantic-errors main.c -std=gnu99
	
utils.o: utils.c
	gcc -c -Wall -pedantic-errors utils.c -std=gnu99

clean:
	rm -rf *o