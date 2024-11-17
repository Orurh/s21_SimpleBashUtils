#include "catFlags.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
void PrintSpecialChar(int c, CatFlags *flags) {
  if (flags->show_tabs && c == '\t')
    printf("^I");
  else if (flags->show_hiddensimv && ((c < 32 && c != '\n' && c != '\t')))
    printf("^%c", c + 64);
  else if (flags->show_hiddensimv && c == 127)
    printf("^?");
  else if (flags->show_hiddensimv && c >= 128 && c < 160)
    printf("M-^%c", c - 128 + 64);
  else if (flags->show_ends && c == 0)
    printf("^@");
  else if (flags->show_ends && c == '\n')
    printf("$\n");
  else
    putchar(c);
}

void processFile(FILE *fp, CatFlags *flags) {
    int c;
    int previous = '\n';  // Начинаем с символа новой строки
    int index_nonblank = 1;  // Начинаем нумерацию ненулевых строк
    int line_not_empty = 0;  // Флаг, указывающий, есть ли содержимое в строке

    while ((c = fgetc(fp)) != EOF) {
        // Логика для squeeze_blank
        // if (flags->squeeze_blank && c == '\n' && previous == '\n') {
        //     continue;  // Пропускаем, если предыдущий символ также новая строка
        // }

        // Проверяем наличие содержимого в строке
        if (c != '\n') {
            line_not_empty = 1;  // У нас есть видимый символ
        }

        // Нумерация только ненулевых строк
        if ((line_not_empty && flags->number_nonblanck && previous == '\n') || (flags->number_all && previous == '\n')) {

                printf("%d\t", index_nonblank++);

        }

        // Печать специального символа
        PrintSpecialChar(c, flags);
        
        // Обновляем предыдущий символ
        previous = c;
        
        // Если текущий символ - новая строка, сбрасываем флаг
        if (c == '\n') {
            line_not_empty = 0;  // Сбрасываем флаг для следующей строки
        }
    }
}