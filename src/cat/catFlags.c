#include "catFlags.h"

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
    printf("M-^%c", c + 64);
  else if (flags->show_ends && c == '\n')
    printf("$\n");
  else
    putchar(c);
}

void ProcessFile(FILE *fp, const CatFlags *flags, int *index, int *previous) {
  int c;
  int line_count = 0;

  while ((c = fgetc(fp)) != EOF) {
    if (flags->squeeze_blank && c == '\n' && *previous == '\n') {
      line_count++;
    } else {
      line_count = 0;
    }

    if (!(flags->squeeze_blank && line_count > 1)) {
      if ((flags->number_all || (flags->number_nonblanck && c != '\n')) &&
          *previous == '\n') {
        printf("%6d\t", (*index)++);
      }
      PrintSpecialChar(c, flags);
    }
    *previous = c;
  }
}