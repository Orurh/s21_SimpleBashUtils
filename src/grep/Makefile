FLAGS=-Wall -Wextra -Werror -std=c11 -pedantic 

CC=gcc

all: s21_grep

s21_grep: s21_grep.o
	$(CC) ${FLAGS} s21_grep.o -o s21_grep

s21_grep.o: s21_grep.c
	$(CC) ${FLAGS} -c s21_grep.c
run:
	./test.sh

clean:
	rm -rf *.o s21_grep

rebuild:  all