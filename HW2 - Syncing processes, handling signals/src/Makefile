all: p1.o p2.o utils.o
	gcc -Wall p1.o utils.o -lm -o p1
	gcc -Wall p2.o utils.o -lm -o p2

p1.o: p1.c
	gcc -c -Wall -pedantic-errors p1.c -std=gnu99 

p2.o: p2.c
	gcc -c -Wall  -pedantic-errors p2.c -std=gnu99

utils.o: utils.c 
	gcc -c -Wall -pedantic-errors utils.c -std=gnu99

clean:
	rm -rf *o