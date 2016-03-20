CFLAGS = -Wall -g

polls: polls.o lists.o 
	gcc $(CFLAGS) -o polls polls.o lists.o

polls.o: polls.c lists.h
	gcc $(CFLAGS) -c polls.c

lists.o: lists.c lists.h
	gcc $(CFLAGS) -c lists.c

clean: 
	rm polls *.o