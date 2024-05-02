all:
	gcc main.c -o programa -lpthread

run:
	./programa 10 2 3 3 3 5 7