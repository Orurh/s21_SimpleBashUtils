#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e_flag;  // Использование регулярного выражения
  int i_flag;  // Игнорировать регистр
  int v_flag;  // Инвертировать результат поиска
  int c_flag;  // Подсчитать количество совпадений
  int l_flag;  // Показать имя файлов с совпадениями
  int n_flag;  // Показать номер строки
  int h_flag;  // Не показывать имена файлов
  int s_flag;  // Молчание, игнорировать ошибки при открытии файлов
  int f_flag;  // Чтение паттернов из файла
  int o_flag;  // Выводить только совпавшие части строки
  char *pattern;
  char *pattern_file;
  int pattern_count;
  int filename_flag;
  regex_t *regexes;  // Массив регулярных выражений
  int regex_count;   // Количество паттернов
} GrepFlags;

void InitializeFlags(GrepFlags *flags) {
  *flags = (GrepFlags){0};  // Инициализация всех полей в ноль
  flags->regexes = NULL;
  flags->regex_count = 0;
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

void HandleFlagF(GrepFlags *flags, char *optarg, int *return_value) {
  flags->f_flag = 1;
  if (optarg) {
    flags->pattern_file = optarg;
  } else {
    *return_value = 1;
    fprintf(stderr, "Error: Option -f requires a file argument.\n");
  }
}

void CompilePattern(GrepFlags *flags, int *return_value) {
  if (!flags->pattern && !flags->pattern_file) {
    fprintf(stderr, "Error: No search pattern provided.\n");
    *return_value = 1;
    return;
  }

  int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);

  // Если паттерн передан через -e
  if (flags->pattern) {
    regex_t regex;
    if (regcomp(&regex, flags->pattern, regex_flags) != 0) {
      fprintf(stderr, "Error: Invalid regular expression.\n");
      *return_value = 1;
      return;
    }
    // Добавляем в массив регулярных выражений
    flags->regexes =
        realloc(flags->regexes, sizeof(regex_t) * (flags->regex_count + 1));
    flags->regexes[flags->regex_count] = regex;
    flags->regex_count++;
  }

  // Если паттерны идут из файла -f
  if (flags->pattern_file) {
    FILE *file = fopen(flags->pattern_file, "r");
    if (!file) {
      fprintf(stderr, "Error: Could not open pattern file: %s\n",
              flags->pattern_file);
      *return_value = 1;
      return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
      // Удаление символа новой строки
      line[strcspn(line, "\n")] = 0;

      regex_t regex;
      if (regcomp(&regex, line, regex_flags) != 0) {
        fprintf(stderr, "Error: Invalid regular expression in file: %s\n",
                line);
        fclose(file);
        *return_value = 1;
        return;
      }

      // Добавляем в массив регулярных выражений
      flags->regexes =
          realloc(flags->regexes, sizeof(regex_t) * (flags->regex_count + 1));
      flags->regexes[flags->regex_count] = regex;
      flags->regex_count++;
    }
    fclose(file);
  }
}

void ParseArguments(GrepFlags *flags, int argc, char *argv[],
                    int *return_value) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
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
      case 'f':
        HandleFlagF(flags, optarg, return_value);
        break;
      case 'o':
        flags->o_flag = 1;
        break;
      default:
        *return_value = 1;
        fprintf(stderr, "Error: Unknown option -%c\n", opt);
        return;
    }
  }

  if (!flags->e_flag && !flags->f_flag && optind < argc) {
    flags->pattern = argv[optind++];
    flags->pattern_count++;
  }

  CompilePattern(flags, return_value);
}

FILE *OpenFile(const char *filename, GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

void ProcessLine(const char *line, int line_number, GrepFlags *flags,
                 int *match_count, const char *filename, int *file_matched,
                 int total_files) {
  int match = 0;

  for (int i = 0; i < flags->regex_count; i++) {
    match = regexec(&flags->regexes[i], line, 0, NULL, 0) == 0;
    if (flags->v_flag) {
      match = !match;
    }

    if (match) {
      if (flags->o_flag) {
        regmatch_t matches[10];
        if (regexec(&flags->regexes[i], line, 10, matches, 0) == 0) {
          for (int j = 0; j < 10 && matches[j].rm_so != -1; j++) {
            // Добавляем номер строки перед совпадением
            if (flags->n_flag) {
              printf("%d:", line_number);
            }
            printf("%.*s\n", matches[j].rm_eo - matches[j].rm_so,
                   line + matches[j].rm_so);
          }
        }
      } else {
        if (flags->c_flag) {
          (*match_count)++;
        } else if (flags->l_flag) {
          if (!*file_matched) {  // Выводим имя файла только один раз
            printf("%s\n", filename);
            *file_matched = 1;
          }
        } else {
          if (total_files > 1 &&
              !flags->h_flag) {  // Выводим имя файла, если несколько файлов
            printf("%s:", filename);
          }
          if (flags->n_flag) {
            printf("%d:", line_number);
          }
          printf("%s", line);
        }
      }
      break;  // Если хотя бы одно регулярное выражение совпало
    }
  }
}


void ProcessFile(FILE *file, GrepFlags *flags, const char *filename,
                 int total_files) {
  char line[1024];
  int line_number = 1;
  int match_count = 0;
  int file_matched = 0;  // Флаг, был ли напечатан файл для `-l`

  while (fgets(line, sizeof(line), file)) {
    ProcessLine(line, line_number, flags, &match_count, filename, &file_matched,
                total_files);
    line_number++;
    if (flags->l_flag && file_matched) {
      break;  // Если флаг -l, то выводим имя файла и прекращаем дальнейший
              // поиск
    }
  }

  if (flags->c_flag) {
    printf("%d\n", match_count);
  }
}

void FreeResources(GrepFlags *flags) {
  for (int i = 0; i < flags->regex_count; i++) {
    regfree(&flags->regexes[i]);
  }
  free(flags->regexes);
}

int main(int argc, char *argv[]) {
  int return_value = 0;
  GrepFlags flags;
  InitializeFlags(&flags);

  ParseArguments(&flags, argc, argv, &return_value);
  if (return_value != 0) {
    FreeResources(&flags);
    return EXIT_FAILURE;
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: No files provided.\n");
    FreeResources(&flags);
    return EXIT_FAILURE;
  }

  int total_files = argc - optind;  // Общее количество файлов
  for (int i = optind; i < argc; ++i) {
    const char *filename = argv[i];
    FILE *file = OpenFile(filename, &flags);
    if (file) {
      ProcessFile(file, &flags, filename, total_files);
      fclose(file);
    }
  }

  FreeResources(&flags);
  return return_value;
}
