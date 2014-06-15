CC = gcc
LIBS = -lpthread
VERSION = -std=c99
DEBUG = -g

all:
	$(CC) $(VERSION) threadpool.c -o threadpool-bin $(LIBS)
	./threadpool-bin
debug:
	$(CC) $(VERSION) $(DEBUG) threadpool.c $(LIBS)
	gdb a.out

clean:
	rm -f threadpool-bin a.out
