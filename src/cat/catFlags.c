#include "catFlags.h"

#include <stdio.h>
#include <string.h>

void PrintSpecialChar(int c, CatFlags *flags) {
  if (flags->show_tabs && c == '\t')
    printf("^I");
  else if (flags->show_hiddensimv &&
    ((c < 32 && c != '\n' && c != '\t')))
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

void PrintLineWithOptions(const char *line, int *lineNumber, CatFlags *flags,
                          int *prevEmpty) {
  int is_empty = (line[0] == '\n');

  if (!(flags->squeeze_blank && *prevEmpty && is_empty)) {
    if (flags->number_all && !(flags->number_nonblanck && is_empty)) {
      printf("%6d\t", (*lineNumber)++);
    } else if (flags->number_nonblanck && !is_empty) {
      printf("%6d\t", (*lineNumber)++);
    }

    size_t len = strlen(line);
    for (size_t i = 0; i < len; i++) {
      // unsigned char c = (unsigned char)line[i]t)(unsigned);
      int c = (int)(unsigned char)line[i];
      PrintSpecialChar(c, flags);
    }
  }
  *prevEmpty = is_empty;
}
