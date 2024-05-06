FLAGS = -Wall -Werror -Wpedantic -fsanitize=address -lpthread

all:
	gcc $(FLAGS) main.c -o program

run:
	./program 10 2 1 1 3 5 1
	./program 1 1 1 1 1 1 1
	
clean:
	rm program

