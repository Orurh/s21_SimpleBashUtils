#include "s21_grep.h"

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void InitializeFlags(GrepFlags *flags) { *flags = (GrepFlags){0}; }
FILE *OpenFile(const char *filename, const GrepFlags *flags);

int main(int argc, char *argv[]) {
  int return_value = 0;
  GrepFlags flags;
  InitializeFlags(&flags);
  return_value = ParseArguments(&flags, argc, argv);
  if (!flags.skip_processing && return_value == 0 && flags.error_code == 0) {
    if (optind < argc) {
      int total_files = argc - optind;
      for (int i = optind; i < argc; ++i) {
        const char *filename = argv[i];
        FILE *file = OpenFile(filename, &flags);
        if (file) {
          ProcessFile(file, filename, &flags, total_files);
          fclose(file);
        }
      }
    } else {
      ProcessFile(stdin, "(standard input)", &flags, 1);
    }
  }
  FreeResources(&flags);
  return flags.error_code ? flags.error_code : return_value;
}

void CompilePattern(GrepFlags *flags) {
  if (!flags->combined_pattern) {
    fprintf(stderr, "Error: No valid patterns found.\n");
    flags->error_code = 1;
  }
  int regex_flags = (flags->i_flag ? REG_ICASE : 0);
  if (regcomp(&flags->regex, flags->combined_pattern, regex_flags) != 0) {
    fprintf(stderr, "Error: Invalid regular expression: %s\n",
            flags->combined_pattern);
    flags->error_code = 1;
  }
}

FILE *OpenFile(const char *filename, const GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

int CheckMatch(const char *line, const regex_t *regex, int invert_match) {
  int match = (regexec(regex, line, 0, NULL, 0) == 0);
  return invert_match ? !match : match;
}

// Печать совпадающей строки с деталями

void FreeResources(GrepFlags *flags) {
  regfree(&flags->regex);
  free(flags->combined_pattern);
}
