#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATTERNS 10
#define MAX_LINE_LENGTH 1024

typedef struct {
    int e_flag; // Использование регулярного выражения
    int i_flag; // Игнорировать регистр
    int v_flag; // Инвертировать результат поиска
    int c_flag; // Подсчитать количество совпадений
    int l_flag; // Показать имя файлов с совпадениями
    int n_flag; // Показать номер строки
    int h_flag; // Отключить имя файла
    int s_flag; // Подавить ошибки
    int o_flag; // Вывести только совпадения
    regex_t regexes[MAX_PATTERNS]; // Компилированные паттерны
    char *patterns[MAX_PATTERNS];  // Исходные паттерны
    int pattern_count;             // Количество паттернов
} GrepFlags;

void InitializeFlags(GrepFlags *flags, int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "e:ivclnhso")) != -1) {
        switch (opt) {
        case 'e':
            if (flags->pattern_count < MAX_PATTERNS) {
                flags->e_flag = 1;
                flags->patterns[flags->pattern_count++] = optarg;
            } else {
                fprintf(stderr, "Too many patterns specified.\n");
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
            exit(EXIT_FAILURE);
        }
    }

    if (!flags->e_flag && optind < argc) {
        flags->patterns[flags->pattern_count++] = argv[optind++];
    }

    int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
    for (int i = 0; i < flags->pattern_count; ++i) {
        if (regcomp(&flags->regexes[i], flags->patterns[i], regex_flags) != 0) {
            fprintf(stderr, "Error compiling regex: %s\n", flags->patterns[i]);
            exit(EXIT_FAILURE);
        }
    }
}

int MatchPatterns(const char *line, GrepFlags *flags) {
    for (int i = 0; i < flags->pattern_count; ++i) {
        int match = (regexec(&flags->regexes[i], line, 0, NULL, 0) == 0);
        if (match) return flags->v_flag ? 0 : 1; // Инверсия результата
    }
    return flags->v_flag ? 1 : 0;
}

void ProcessLine(const char *line, int line_number, GrepFlags *flags, int *match_count, const char *filename) {
    if (MatchPatterns(line, flags)) {
        if (flags->c_flag) {
            (*match_count)++;
        } else if (flags->l_flag) {
            printf("%s\n", filename);
        } else {
            if (!flags->h_flag && filename) {
                printf("%s:", filename);
            }
            if (flags->n_flag) {
                printf("%d:", line_number);
            }
            if (flags->o_flag) {
                for (int i = 0; i < flags->pattern_count; ++i) {
                    regmatch_t match;
                    const char *current_line = line;
                    while (regexec(&flags->regexes[i], current_line, 1, &match, 0) == 0) {
                        printf("%.*s\n", (int)(match.rm_eo - match.rm_so), &current_line[match.rm_so]);
                        current_line += match.rm_eo;
                    }
                }
            } else {
                printf("%s", line);
                if (line[strlen(line) - 1] != '\n') {
                    printf("\n");
                }
            }
        }
    }
}

void ProcessFile(FILE *file, GrepFlags *flags, const char *filename) {
    char line[MAX_LINE_LENGTH];
    int line_number = 1;
    int match_count = 0;

    while (fgets(line, sizeof(line), file)) {
        ProcessLine(line, line_number++, flags, &match_count, filename);
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

    for (int i = optind; i < argc; ++i) {
        FILE *file = fopen(argv[i], "r");
        if (!file) {
            if (!flags.s_flag) perror(argv[i]);
            continue;
        }

        ProcessFile(file, &flags, flags.h_flag ? NULL : argv[i]);
        fclose(file);
    }

    for (int i = 0; i < flags.pattern_count; ++i) {
        regfree(&flags.regexes[i]);
    }

    return EXIT_SUCCESS;
}
