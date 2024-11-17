#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <regex.h>

typedef struct {
    int e_flag; // Использование регулярного выражения
    int i_flag; // Игнорировать регистр
    int v_flag; // Инвертировать результат поиска
    int c_flag; // Подсчитать количество совпадений
    int l_flag; // Показать имя файлов с совпадениями
    int n_flag; // Показать номер строки
    regex_t regex;
    char *pattern;
    int pattern_count;
} GrepFlags;

int main(int argc, char const *argv[])
{
    int opt;
    GrepFlags flags = {0};
    while ((opt = getopt(argc, argv, "e:ivcln")) != -1) {
        if (opt == 'e') {
            flags.e_flag = 1;
            flags.pattern = optarg;
        } else if (opt == 'i') {
            flags.i_flag = 1;
        } else if (opt == 'v') {
            flags.v_flag = 1;
        } else if (opt == 'c') {
            flags.c_flag = 1;
        } else if (opt == 'l') {
            flags.l_flag = 1;
        } else if (opt == 'n') {
            flags.n_flag = 1;
        }
    }
    return 0;
}
