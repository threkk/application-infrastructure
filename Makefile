CC = gcc
LIBS = -lpthread
VERSION = -std=c99
OPTIONS = -w -O2

all: clean
	$(CC) $(VERSION) $(OPTIONS) -c threadpool.c -o threadpool.o $(LIBS)
	ar rcs libthreadpool.a threadpool.o
test:all
	$(CC) $(VERSION) $(OPTIONS) -static test.c -L. -lthreadpool $(LIBS) -o test

clean:
	rm -f *.o
	rm -f test
	rm -f libthreadpool.a
