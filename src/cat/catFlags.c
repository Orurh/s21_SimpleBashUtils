#include "catFlags.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
void PrintSpecialChar(int c, const CatFlags *flags) {
  if (flags->show_tabs && c == '\t')
    printf("^I");
  else if (flags->show_hiddensimv && ((c < 32 && c != '\n' && c != '\t')))
    printf("^%c", c + 64);
  else if (flags->show_hiddensimv && c == 127)
    printf("^?");
  else if (flags->show_hiddensimv && c >= 128 && c < 160)
    printf("M-^%c", c - 128 + 64);
  else if (flags->show_ends && c == '\n')
    printf("$\n");
  else
    putchar(c);
}
void processFile(FILE *fp, const CatFlags *flags, int *index) {
  int c;
  int previous = '\n'; // Предыдущий символ
  int blank_count = 0; // Счетчик пустых строк

  while ((c = fgetc(fp)) != EOF) {
    c = (unsigned char)c; // Защита от некорректной интерпретации символов

    // Обработка флага -s (squeeze blank)
    if (flags->squeeze_blank && c == '\n' && previous == '\n') {
      blank_count++;
      if (blank_count > 1) {
        continue; // Пропускаем лишние пустые строки
      }
    } else {
      blank_count = 0; // Сбрасываем счетчик пустых строк
    }
    if (flags->number_all || (flags->number_nonblanck && c != '\n')) {
      if (previous == '\n') {
        printf("%6d\t", (*index)++);
      }
    }
    PrintSpecialChar(c, flags);
    previous = c;
  }
}