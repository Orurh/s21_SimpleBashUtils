#include "s21_grep.h"

void ProcessLine(const char *line, int line_number, GrepFlags *flags,
                 int *match_count, const char *filename) {
  int match = regexec(&flags->regex, line, 0, NULL, 0) == 0;
  if (flags->v_flag) {
    match = !match;
  }

  if (match) {
    if (flags->c_flag) {
      (*match_count)++;
    } else if (flags->l_flag) {
      printf("%s\n", filename);
    } else {
      if (flags->filename_flag && !flags->h_flag) {
        printf("%s:", filename);
      }
      if (flags->n_flag) {
        printf("%d:", line_number);
      }
      printf("%s", line);
    }
  }
}
