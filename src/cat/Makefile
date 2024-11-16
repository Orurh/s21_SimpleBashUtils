FLAGS=-Wall -Wextra -Werror -std=c11 

CC=gcc

all: s21_cat

s21_cat: s21_cat.o catFlags.o
	$(CC) $(FLAGS) s21_cat.o catFlags.o -o s21_cat

s21_cat.o: s21_cat.c
	$(CC) $(FLAGS) -c s21_cat.c

catFlags.o: catFlags.c
	$(CC) $(FLAGS) -c catFlags.c


clean:
	rm -rf *.o s21_cat 

rebuild:  all