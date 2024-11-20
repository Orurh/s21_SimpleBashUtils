#include "s21_grep.h"
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 4098

FILE *OpenFile(const char *filename, const GrepFlags *flags);

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

// Компиляция одной регулярки
int CompileSinglePattern(const char *pattern, int regex_flags,
                         GrepFlags *flags) {
  int return_value = 0;
  regex_t regex;
  if (regcomp(&regex, pattern, regex_flags) != 0) {
    fprintf(stderr, "Error: Invalid regular expression: %s\n", pattern);
    return_value = 1;
  }
  flags->regexes =
      realloc(flags->regexes, sizeof(regex_t) * (flags->regex_count + 1));
  if (!flags->regexes) {
    perror("Memory allocation error");
    return_value = 1;
  }
  flags->regexes[flags->regex_count++] = regex;
  return return_value;
}

// Компиляция всех регулярных выражений
void CompilePattern(GrepFlags *flags, int *return_value) {
  int regex_flags = (flags->i_flag ? REG_ICASE : 0);

  if (flags->pattern) {
    if (CompileSinglePattern(flags->pattern, regex_flags, flags)) {
      *return_value = 1;
    }
  }
  if (flags->pattern_file) {
    FILE *file = OpenFile(flags->pattern_file, flags);
    if (file) {
      char line[LINE_BUFFER_SIZE];
      while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (CompileSinglePattern(line, regex_flags, flags)) {
          *return_value = 1;
        }
      }
      fclose(file);
    } else {
      fprintf(stderr, "Error: Could not open pattern file: %s\n",
              flags->pattern_file);
      *return_value = 1;
    }
  }
  if (!flags->regex_count) {
    fprintf(stderr, "Error: No valid patterns found.\n");
    *return_value = 1;
  }
}

// Применение приоритетов флагов
void ApplyFlagsPriority(GrepFlags *flags) {
  if (flags->l_flag) {
    flags->c_flag = flags->o_flag = flags->n_flag = 0;
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
    }
  }
  if (!flags->e_flag && !flags->f_flag && optind < argc) {
    flags->pattern = argv[optind++];
    flags->pattern_count++;
  }
  CompilePattern(flags, return_value);
  ApplyFlagsPriority(flags);
}

// Открытие файла
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

void PrintMatchDetails(const char *line, int line_number,
                       const GrepFlags *flags, const char *filename,
                       int total_files) {
  if (total_files > 1 && !flags->h_flag) {
    printf("%s:", filename);
  }
  if (flags->n_flag) {
    printf("%d:", line_number);
  }
  if (flags->o_flag) {
    regmatch_t matches[10];
    if (regexec(&flags->regexes[0], line, 10, matches, 0) == 0) {
      for (int j = 0; j < 10 && matches[j].rm_so != -1; j++) {
        printf("%.*s\n", matches[j].rm_eo - matches[j].rm_so,
               line + matches[j].rm_so);
      }
    }
  } else {
    printf("%s", line);
  }
}

void ProcessMatch(const char *line, int line_number, const GrepFlags *flags,
                  const char *filename, int total_files, int *match_count,
                  int *file_matched, int *stop_processing) {
  if (flags->l_flag) {
    if (!*file_matched) {
      printf("%s\n", filename);
      *file_matched = 1;
    }
    *stop_processing = 1; // Прекращаем обработку файла
  } else if (flags->c_flag) {
    (*match_count)++;
  } else {
    PrintMatchDetails(line, line_number, flags, filename, total_files);
  }
}

int ProcessLine(const char *line, int line_number, const GrepFlags *flags,
                const char *filename, int total_files, int *match_count,
                int *file_matched) {
  int stop_processing = 0;

  for (int i = 0; i < flags->regex_count && !stop_processing; i++) {
    int match = CheckMatch(line, &flags->regexes[i], flags->v_flag);
    if (match) {
      ProcessMatch(line, line_number, flags, filename, total_files, match_count,
                   file_matched, &stop_processing);
    }
  }

  return stop_processing;
}

// Обработка файла
void ProcessFile(FILE *file, const char *filename, const GrepFlags *flags,
                 int total_files) {
  char line[LINE_BUFFER_SIZE];
  int line_number = 1;
  int match_count = 0;
  int file_matched = 0;

  while (fgets(line, sizeof(line), file)) {
    if (ProcessLine(line, line_number++, flags, filename, total_files,
                    &match_count, &file_matched)) {
      break;
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
  if (!return_value || optind < argc) {
    int total_files = argc - optind;
    for (int i = optind; i < argc; ++i) {
      const char *filename = argv[i];
      FILE *file = OpenFile(filename, &flags);
      if (file) {
        ProcessFile(file, filename, &flags, total_files);
        fclose(file);
      }
    }
  } else if (optind >= argc) {
    fprintf(stderr, "Error: No files provided.\n");
  }
  FreeResources(&flags);
  return return_value;
}
