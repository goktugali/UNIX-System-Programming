all: parent.o child.o utils.o 
	gcc -Wall parent.o utils.o -lm -o parent
	gcc -Wall child.o utils.o -lm -o child

child.o : child.c
	gcc -c -Wall -pedantic-errors child.c -std=gnu99

parent.o: parent.c
	gcc -c -Wall -pedantic-errors parent.c -std=gnu99 

utils.o: utils.c 
	gcc -c -Wall -pedantic-errors utils.c -std=gnu99

clean:
	rm -rf *o