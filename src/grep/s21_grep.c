#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e_flag; // pattern
  int i_flag; // no register
  int v_flag; // reverse unswer
  int c_flag; // count num of string woth pattern
  int l_flag; //
  int n_flag; // show num of string in file
  regex_t regex;
  char *pattern;
} GrepFlags;

int InitializeFlags(GrepFlags *flags, int argc, char *argv[]) {
  int opt;
  int return_flag = 0;
  while ((opt = getopt(argc, argv, "e:ivcln")) != -1) {
    switch (opt) {
      case 'e':
        flags->e_flag = 1;
        flags->pattern = optarg;
        break;
      case 'i':
        flags->i_flag = 1;
        break;
      case 'v':
        flags->v_flag = 1;
        break;
      case 'c':
        flags->c_flag = 1;
        break;
      case 'l':
        flags->l_flag = 1;
        break;
      case 'n':
        flags->n_flag = 1;
        flags->e_flag = 1;
        break;
      default:
        return_flag = EXIT_FAILURE;
    }
  }

  int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
  if (regcomp(&flags->regex, flags->pattern, regex_flags) != 0) {
    fprintf(stderr, "Ошибка компиляции регулярного выражения.\n");
    return_flag = EXIT_FAILURE;
  }
  return return_flag;
}

int MatchLine(const char *line, GrepFlags *flags) {
  int match = (regexec(&flags->regex, line, 0, NULL, 0) == 0);
  return flags->v_flag ? !match : match;
}

void PrintLine(const char *line, int line_number, GrepFlags *flags,
               int *match_count, const char *filename) {
  if (flags->c_flag) {
    (*match_count)++;
  } else if (flags->l_flag) {
    printf("%s\n", filename);
  } else {
    if (flags->n_flag) {
      printf("%d:", line_number);
    }
    printf("%s", line);
  }
}

void ProcessFile(FILE *file, GrepFlags *flags, const char *filename) {
  char line[1024];
  int line_number = 1;
  int match_count = 0;

  while (fgets(line, sizeof(line), file)) {
    if (MatchLine(line, flags)) {
      PrintLine(line, line_number, flags, &match_count, filename);
    }
    line_number++;
  }

  if (flags->c_flag) {
    printf("%d\n", match_count);
  }
}

int main(int argc, char *argv[]) {
  GrepFlags flags = {0};
  int return_flag = InitializeFlags(&flags, argc, argv);

  FILE *file = fopen(argv[optind], "r");
  if (!file) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  ProcessFile(file, &flags, argv[optind]);

  fclose(file);
  regfree(&flags.regex);
  return return_flag == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
