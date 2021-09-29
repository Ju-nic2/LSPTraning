mytop : mytop.o
	gcc -o mytop mytop.o -lpthread -lncurses -lm

mytop.o : myheader.h mytop.c
	gcc -c  mytop.c myheader.h 
