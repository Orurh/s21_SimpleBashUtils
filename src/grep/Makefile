FLAGS=-Wall -Wextra -Werror -std=c11 -pedantic 

CC=gcc

all: s21_grep

s21_grep: s21_grep.o
	$(CC) s21_grep.o -o s21_grep

s21_grep.o: s21_grep.c
	$(CC) -c s21_grep.c
run:
	./test.sh

clean:
	rm -rf *.o s21_grep

rebuild:  all