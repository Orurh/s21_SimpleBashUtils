#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e_flag; // Использование регулярного выражения
  int i_flag; // Игнорировать регистр
  int v_flag; // Инвертировать результат поиска
  int c_flag; // Подсчитать количество совпадений
  int l_flag; // Показать имя файлов с совпадениями
  int n_flag; // Показать номер строки
  int h_flag; // Не показывать имена файлов
  int s_flag; // Молчание, игнорировать ошибки при открытии файлов
  regex_t regex;
  char *pattern;
  int pattern_count;
  int filename_flag;
} GrepFlags;

void InitializeFlags(GrepFlags *flags) {
  *flags = (GrepFlags){0}; // Инициализация всех полей в ноль
}

void HandleFlagE(GrepFlags *flags, char *optarg, int *return_value) {
  flags->e_flag = 1;
  if (optarg) {
    flags->pattern = optarg;
    flags->pattern_count++;
  } else {
    *return_value = 1;
    fprintf(stderr, "Error: Option -e requires an argument.\n");
  }
}

void CompilePattern(GrepFlags *flags, int *return_value) {
  if (!flags->pattern) {
    fprintf(stderr, "Error: No search pattern provided.\n");
    *return_value = 1;
    return;
  }

  int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
  if (regcomp(&flags->regex, flags->pattern, regex_flags) != 0) {
    fprintf(stderr, "Error: Invalid regular expression.\n");
    exit(EXIT_FAILURE);
  }
}

void ParseArguments(GrepFlags *flags, int argc, char *argv[],
                    int *return_value) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhs")) != -1) {
    switch (opt) {
    case 'e':
      HandleFlagE(flags, optarg, return_value);
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
    case 'h':
      flags->h_flag = 1;
      break;
    case 's':
      flags->s_flag = 1;
      break;
    default:
      *return_value = 1;
      fprintf(stderr, "Error: Unknown option -%c\n", opt);
      return;
    }
  }

  if (!flags->e_flag && optind < argc) {
    flags->pattern = argv[optind++];
    flags->pattern_count++;
  }

  CompilePattern(flags, return_value);
  flags->filename_flag = (argc - optind > 1);
}

FILE *OpenFile(const char *filename, GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

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

void FreeResources(GrepFlags *flags) { regfree(&flags->regex); }

int main(int argc, char *argv[]) {
  int return_value = 0;
  GrepFlags flags;
  InitializeFlags(&flags);

  ParseArguments(&flags, argc, argv, &return_value);
  if (return_value != 0) {
    return EXIT_FAILURE;
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: No files provided.\n");
    FreeResources(&flags);
    return EXIT_FAILURE;
  }

  for (int i = optind; i < argc; ++i) {
    const char *filename = argv[i];
    FILE *file = OpenFile(filename, &flags);
    if (file) {
      ProcessFile(file, &flags, filename);
      fclose(file);
    }
  }

  FreeResources(&flags);
  return return_value;
}
