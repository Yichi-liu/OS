CFLAGS += -g -Wall -Wextra -Wpedantic

tabstop: tabstop.o problem03.o
	gcc $(CFLAGS) -o tabstop tabstop.o problem03.o

tabstop.o: tabstop.c problem03.h
	gcc $(CFLAGS) -c tabstop.c

problem03.o: problem03.c problem03.h
	gcc $(CFLAGS) -c problem03.c

clean:
	rm -f *.exe *.o *.stackdumpS
