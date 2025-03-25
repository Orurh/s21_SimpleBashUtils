FLAGS=-Wall -Wextra -Werror -std=c11 -pedantic

CC=gcc

all: s21_grep

s21_grep: s21_grep.o s21_parser.o s21_process.o
	$(CC) ${FLAGS} s21_grep.o s21_parser.o s21_process.o -o s21_grep

s21_grep.o: s21_grep.c
	$(CC) ${FLAGS} -c s21_grep.c

s21_parser.o: s21_parser.c
	$(CC) ${FLAGS} -c s21_parser.c

s21_process.o: s21_process.c
	$(CC) ${FLAGS} -c s21_process.c

run:
	./test.sh

clean:
	rm -rf *.o s21_grep

rebuild:  all