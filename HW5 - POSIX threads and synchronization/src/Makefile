all: main.o utils.o
	gcc -Wall main.o utils.o -lrt -lpthread -o floristApp -lm
	@echo "\n> Run with : ./floristApp -i filepath"

main.o: main.c
	gcc -c -Wall -Werror -pedantic-errors main.c -std=gnu99 -g -lm
	
utils.o: utils.c utils.h
	gcc -c -Wall -Werror -pedantic-errors utils.c -std=gnu99 -g -lm

clean:
	rm -rf *o