#include "s21_grep.h"

void ProcessFile(FILE *file, const char *filename, const GrepFlags *flags,
                 int total_files) {
  char line[4098];
  int line_number = 1;
  int match_count = 0;
  int file_matched = 0;
  int stop_processing = 0;

  while (!stop_processing && fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';
    stop_processing = ProcessLine(line, line_number++, flags, filename,
                                  total_files, &match_count, &file_matched);
  }

  if (flags->c_flag) {
    if (total_files > 1 && !flags->h_flag) {
      printf("%s:", filename);
    }
    printf("%d\n", match_count);
  }
}
int ProcessLine(const char *line, int line_number, const GrepFlags *flags,
                const char *filename, int total_files, int *match_count,
                int *file_matched) {
  int stop_processing = 0;
  int match = CheckMatch(line, &flags->regex, flags->v_flag);
  if (match) {
    if (flags->l_flag) {
      if (!*file_matched) {
        printf("%s\n", filename);
        *file_matched = 1;
      }
      stop_processing = 1;
    } else if (flags->c_flag)
      (*match_count)++;
    else
      PrintMatchDetails(line, line_number, flags, filename, total_files);
  }
  return stop_processing;
}

void PrintMatchDetails(const char *line, int line_number,
                       const GrepFlags *flags, const char *filename,
                       int total_files) {
  if (total_files > 1 && !flags->h_flag) {
    if (!flags->o_flag) printf("%s:", filename);
  }
  if (flags->n_flag && !flags->o_flag) {
    printf("%d:", line_number);
  }
  if (flags->o_flag) {
    regmatch_t match;

    const char *ptr = line;
    while (regexec(&flags->regex, ptr, 1, &match, 0) == 0) {
      if (!flags->h_flag && total_files > 1) printf("%s:", filename);
      if (flags->n_flag) printf("%d:", line_number);
      printf("%.*s\n", (int)(match.rm_eo - match.rm_so), ptr + match.rm_so);
      ptr += match.rm_eo;
    }
  } else {
    printf("%s\n", line);
  }
}