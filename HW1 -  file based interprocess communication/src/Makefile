all: programA.o programB.o utils.o fft.o complex.o
	gcc -Wall programA.o utils.o fft.o complex.o -lm -o programA
	gcc -Wall programB.o utils.o fft.o complex.o -lm -o programB

programA.o: programA.c
	gcc -c -Wall -pedantic-errors programA.c -std=gnu99 

programB.o: programB.c
	gcc -c -Wall  -pedantic-errors programB.c -std=gnu99

utils.o: utils.c 
	gcc -c -Wall -pedantic-errors utils.c -std=gnu99

fft.o: fft.c
	gcc -c -Wall -pedantic-errors fft.c -std=gnu99

complex.o: complex.c
	gcc -c -Wall -pedantic-errors complex.c -std=gnu99

clean:
	rm -rf *o