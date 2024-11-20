#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

typedef struct {
  unsigned e_flag : 1;  // Использование регулярного выражения
  unsigned i_flag : 1;  // Игнорировать регистр
  unsigned v_flag : 1;  // Инвертировать результат поиска
  unsigned c_flag : 1;  // Подсчитать количество совпадений
  unsigned l_flag : 1;  // Показать имя файлов с совпадениями
  unsigned n_flag : 1;  // Показать номер строки
  unsigned h_flag : 1;  // Не показывать имена файлов
  unsigned s_flag : 1;  // Молчание, игнорировать ошибки при открытии файлов
  unsigned f_flag : 1;  // Чтение паттернов из файла
  unsigned o_flag : 1;  // Выводить только совпавшие части строки
} GrepFlags;

typedef struct {
  GrepFlags flags;
  regex_t *regexes;
  size_t regex_count;
} GrepContext;

void InitializeContext(GrepContext *ctx) {
  memset(ctx, 0, sizeof(GrepContext));
}

int CompilePattern(GrepContext *ctx, const char *pattern, int regex_flags) {
  regex_t regex;
  if (regcomp(&regex, pattern, regex_flags) != 0) {
    fprintf(stderr, "Error: Invalid regular expression: %s\n", pattern);
    return 1;
  }

  void *temp = realloc(ctx->regexes, sizeof(regex_t) * (ctx->regex_count + 1));
  if (!temp) {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    regfree(&regex);
    return 1;
  }
  ctx->regexes = temp;
  ctx->regexes[ctx->regex_count++] = regex;
  return 0;
}

int ParsePatternsFromFile(GrepContext *ctx, const char *filename,
                          int regex_flags) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open pattern file: %s\n", filename);
    return 1;
  }

  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0;  // Удаление символа новой строки
    if (CompilePattern(ctx, line, regex_flags)) {
      fclose(file);
      return 1;
    }
  }
  fclose(file);
  return 0;
}

int ParseArguments(GrepContext *ctx, int argc, char *argv[], char **files,
                   int *file_count) {
  int regex_flags = REG_EXTENDED | (ctx->flags.i_flag ? REG_ICASE : 0);
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'e':
        ctx->flags.e_flag = 1;
        if (CompilePattern(ctx, optarg, regex_flags)) return 1;
        break;
      case 'i':
        ctx->flags.i_flag = 1;
        break;
      case 'v':
        ctx->flags.v_flag = 1;
        break;
      case 'c':
        ctx->flags.c_flag = 1;
        break;
      case 'l':
        ctx->flags.l_flag = 1;
        break;
      case 'n':
        ctx->flags.n_flag = 1;
        break;
      case 'h':
        ctx->flags.h_flag = 1;
        break;
      case 's':
        ctx->flags.s_flag = 1;
        break;
      case 'f':
        ctx->flags.f_flag = 1;
        if (ParsePatternsFromFile(ctx, optarg, regex_flags)) return 1;
        break;
      case 'o':
        ctx->flags.o_flag = 1;
        break;
      default:
        fprintf(stderr, "Error: Unknown option -%c\n", opt);
        return 1;
    }
  }

  *file_count = argc - optind;
  if (*file_count <= 0) {
    fprintf(stderr, "Error: No files provided.\n");
    return 1;
  }

  *files = argv + optind;
  return 0;
}
void FreeResources(GrepContext *ctx) {
  for (size_t i = 0; i < ctx->regex_count; i++) {
    regfree(&ctx->regexes[i]);
  }
  free(ctx->regexes);
}

FILE *OpenFile(const char *filename, const GrepFlags *flags) {
  FILE *file = fopen(filename, "r");
  if (!file && !flags->s_flag) {
    perror(filename);
  }
  return file;
}

void ProcessLine(const char *line, int line_number, const GrepContext *ctx,
                 int *match_count, const char *filename, int total_files,
                 int *file_matched) {
  int matched = 0;

  for (size_t i = 0; i < ctx->regex_count; i++) {
    matched = regexec(&ctx->regexes[i], line, 0, NULL, 0) == 0;
    if (ctx->flags.v_flag) matched = !matched;
    if (matched) break;
  }

  if (!matched) return;

  if (ctx->flags.c_flag) {
    (*match_count)++;
  } else if (ctx->flags.l_flag) {
    if (!*file_matched) {
      printf("%s\n", filename);
      *file_matched = 1;
    }
  } else {
    if (total_files > 1 && !ctx->flags.h_flag) printf("%s:", filename);
    if (ctx->flags.n_flag) printf("%d:", line_number);
    if (ctx->flags.o_flag) {
      for (size_t i = 0; i < ctx->regex_count; i++) {
        regmatch_t match;
        if (regexec(&ctx->regexes[i], line, 1, &match, 0) == 0) {
          printf("%.*s\n", (int)(match.rm_eo - match.rm_so), line + match.rm_so);
        }
      }
    } else {
      printf("%s", line);
    }
  }
}

void ProcessFile(FILE *file, const char *filename, const GrepContext *ctx,
                 int total_files) {
  char line[MAX_LINE_LENGTH];
  int line_number = 1;
  int match_count = 0;
  int file_matched = 0;

  while (fgets(line, sizeof(line), file)) {
    ProcessLine(line, line_number++, ctx, &match_count, filename, total_files,
                &file_matched);
    if (ctx->flags.l_flag && file_matched) break;
  }

  if (ctx->flags.c_flag) {
    if (total_files > 1 && !ctx->flags.h_flag) printf("%s:", filename);
    printf("%d\n", match_count);
  }
}

void ProcessFiles(GrepContext *ctx, char **files, int file_count) {
  for (int i = 0; i < file_count; ++i) {
    const char *filename = files[i];
    FILE *file = OpenFile(filename, &ctx->flags);
    if (file) {
      ProcessFile(file, filename, ctx, file_count);
      fclose(file);
    }
  }
}

int main(int argc, char *argv[]) {
  GrepContext ctx;
  char **files;
  int file_count;

  InitializeContext(&ctx);

  if (ParseArguments(&ctx, argc, argv, &files, &file_count)) {
    FreeResources(&ctx);
    return EXIT_FAILURE;
  }

  ProcessFiles(&ctx, files, file_count);

  FreeResources(&ctx);
  return EXIT_SUCCESS;
}
