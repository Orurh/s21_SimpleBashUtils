#include "s21_grep.h"

FILE *OpenFile(const char *filename, GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

void ProcessFile(FILE *file, GrepFlags *flags, const char *filename) {
  char line[1024];
  int line_number = 1;
  int match_count = 0;

  while (fgets(line, sizeof(line), file)) {
    ProcessLine(line, line_number, flags, &match_count, filename);
    line_number++;
  }

  if (flags->c_flag) {
    printf("%d\n", match_count);
  }
}
