#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATTERNS 3
typedef struct {
  int e_flag; // Использование регулярного выражения
  int i_flag; // Игнорировать регистр
  int v_flag; // Инвертировать результат поиска
  int c_flag; // Подсчитать количество совпадений
  int l_flag; // Показать имя файлов с совпадениями
  int n_flag; // Показать номер строки
  regex_t regex[MAX_PATTERNS];
  char *pattern[MAX_PATTERNS];
  int pattern_count;
} GrepFlags;

void InitializeFlags(GrepFlags *flags, int argc, char *argv[]) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivcln")) != -1) {
    switch (opt) {
    case 'e':
      if (flags->pattern_count < MAX_PATTERNS) {
        flags->e_flag = 1;
        flags->pattern[flags->pattern_count++] = optarg;
      } else {
        fprintf(stderr, "Слишком много регулярных выражений.\n");
        exit(EXIT_FAILURE);
      }
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
      break;
    default:
      exit(EXIT_FAILURE);
    }
  }
  if (!flags->e_flag) {
    flags->pattern[flags->pattern_count++] = argv[optind++];
  }
  int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
  for (int i = 0; i < flags->pattern_count; i++) {
    if (regcomp(&flags->regex[i], flags->pattern[i], regex_flags) != 0) {
      fprintf(stderr, "Ошибка компиляции регулярного выражения.\n");
      exit(EXIT_FAILURE);
    }
  }
}

int MatchLine(const char *line, GrepFlags *flags) {
  for (int i = 0; i < flags->pattern_count; i++) {
    int match = (regexec(&flags->regex[i], line, 0, NULL, 0) == 0);
    if (match) {
      return flags->v_flag ? !match : match;
    }
  }
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

  void ProcessFile(FILE * file, GrepFlags * flags, const char *filename) {
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
    InitializeFlags(&flags, argc, argv);

      if (optind >= argc) {
        fprintf(stderr, "Error: No file provided.\n");
        return EXIT_FAILURE;
    }

    const char *filename = argv[optind];
    printf("Opening file: %s\n", filename);

    FILE *file = fopen(argv[optind], "r");
    if (!file) {
      perror("fopen");
      return EXIT_FAILURE;
    }

    ProcessFile(file, &flags, argv[optind]);

    fclose(file);
    for (int i = 0; i < flags.pattern_count; ++i) {
      regfree(&flags.regex[i]);
    }
    return EXIT_SUCCESS;
  }
