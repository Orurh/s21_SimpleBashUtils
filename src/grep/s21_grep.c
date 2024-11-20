#include "s21_grep.h"
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void InitializeFlags(GrepFlags *flags) {
  *flags = (GrepFlags){0};
  flags->regexes = NULL;
  flags->regex_count = 0;
}

void HandleFlagEorF(GrepFlags *flags, char *optarg, int *return_value,
                    int opt) {
  if (!optarg) {
    *return_value = 1;
    fprintf(stderr, "Error: Option -%c requires an argument.\n", opt);
  } else {
    if (opt == 'e') {
      flags->e_flag = 1;
      flags->pattern = optarg;
      flags->pattern_count++;
    } else {
      flags->f_flag = 1;
      flags->pattern_file = optarg;
    }
  }
}

// Компиляция регулярных выражений
void CompilePattern(GrepFlags *flags, int *return_value) {
  if (!flags->pattern && !flags->pattern_file) {
    fprintf(stderr, "Error: No search pattern provided.\n");
    *return_value = 1;
    return;
  }

  int regex_flags = (flags->i_flag ? REG_ICASE : 0);

  // Компиляция паттерна из -e
  if (flags->pattern) {
    regex_t regex;
    if (regcomp(&regex, flags->pattern, regex_flags) != 0) {
      fprintf(stderr, "Error: Invalid regular expression.\n");
      *return_value = 1;
      return;
    }
    flags->regexes =
        realloc(flags->regexes, sizeof(regex_t) * (flags->regex_count + 1));
    flags->regexes[flags->regex_count++] = regex;
  }

  // Компиляция паттернов из файла (-f)
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
      line[strcspn(line, "\n")] = 0; // Удаление символа новой строки
      regex_t regex;
      if (regcomp(&regex, line, regex_flags) != 0) {
        fprintf(stderr, "Error: Invalid regular expression in file: %s\n",
                line);
        fclose(file);
        *return_value = 1;
        return;
      }
      flags->regexes =
          realloc(flags->regexes, sizeof(regex_t) * (flags->regex_count + 1));
      flags->regexes[flags->regex_count++] = regex;
    }
    fclose(file);
  }
}

// Применение приоритетов флагов
void ApplyFlagsPriority(GrepFlags *flags) {
  if (flags->l_flag) {
    flags->c_flag = 0;
    flags->o_flag = 0;
    flags->n_flag = 0;
  }
  if (flags->c_flag) {
    flags->o_flag = 0;
  }
}

// Парсинг аргументов
void ParseArguments(GrepFlags *flags, int argc, char *argv[],
                    int *return_value) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
    case 'e':
    case 'f':
      HandleFlagEorF(flags, optarg, return_value, opt);
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
    case 'o':
      flags->o_flag = 1;
      break;
    default:
      *return_value = 1;
      fprintf(stderr, "Error: Unknown option -%c\n", opt);
      return;
    }
  }

  if (!flags->pattern && !flags->pattern_file && optind < argc) {
    flags->pattern = argv[optind++];
    flags->pattern_count++;
  }
  ApplyFlagsPriority(flags);
  CompilePattern(flags, return_value);
}

// Открытие файла
FILE *OpenFile(const char *filename, GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

// Обработка строки
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
      if (flags->l_flag) {
        if (!*file_matched) {
          printf("%s\n", filename);
          *file_matched = 1;
        }
        break;
      }

      if (flags->c_flag) {
        (*match_count)++;
      } else {
        if (total_files > 1 && !flags->h_flag) {
          printf("%s:", filename);
        }
        if (flags->n_flag) {
          printf("%d:", line_number);
        }
        if (flags->o_flag) {
          regmatch_t matches[10];
          if (regexec(&flags->regexes[i], line, 10, matches, 0) == 0) {
            for (int j = 0; j < 10 && matches[j].rm_so != -1; j++) {
              printf("%.*s\n", matches[j].rm_eo - matches[j].rm_so,
                     line + matches[j].rm_so);
            }
          }
        } else {
          printf("%s", line);
        }
      }
      break;
    }
  }
}

// Обработка файла
void ProcessFile(FILE *file, GrepFlags *flags, const char *filename,
                 int total_files) {
  char line[1024];
  int line_number = 1;
  int match_count = 0;
  int file_matched = 0;

  while (fgets(line, sizeof(line), file)) {
    ProcessLine(line, line_number++, flags, &match_count, filename,
                &file_matched, total_files);
    if (flags->l_flag && file_matched) {
      break;
    }
  }

  if (flags->c_flag) {
    printf("%d\n", match_count);
  }
}

// Освобождение ресурсов
void FreeResources(GrepFlags *flags) {
  for (int i = 0; i < flags->regex_count; i++) {
    regfree(&flags->regexes[i]);
  }
  free(flags->regexes);
}

// Главная функция
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

  int total_files = argc - optind;
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
