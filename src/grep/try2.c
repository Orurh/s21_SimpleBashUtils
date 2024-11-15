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
    int l_flag;  // Показать имя файла с совпадениями
    int n_flag;  // Показать номер строки
    int h_flag;  // Не показывать имена файлов
    int s_flag;  // Молчание, игнорировать ошибки при открытии файлов
    int f_flag;  // Использовать файл для шаблонов
    int o_flag;  // Печать только совпадающих частей строки
    regex_t regex;
    char *pattern;
} GrepFlags;

// Функция для безопасного копирования строки
char *copy_string(const char *str) {
    if (!str) return NULL;
    char *copy = malloc(strlen(str) + 1);
    if (!copy) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, str);
    return copy;
}

// Функция загрузки шаблона из файла
int LoadPatternFromFile(const char *filename, char **pattern) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen failed");
        return EXIT_FAILURE;
    }

    size_t size = 0;
    ssize_t read;
    char *line = NULL;
    if ((read = getline(&line, &size, file)) != -1) {
        line[read - 1] = '\0';  // Убираем символ новой строки
        *pattern = copy_string(line);
    }

    free(line);
    fclose(file);
    return (read == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Инициализация флагов
int InitializeFlags(GrepFlags *flags, int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
        switch (opt) {
            case 'e':
                flags->e_flag = 1;
                flags->pattern = copy_string(optarg);
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
                flags->f_flag = 1;
                if (LoadPatternFromFile(optarg, &flags->pattern) != EXIT_SUCCESS) {
                    if (!flags->s_flag) {
                        perror(optarg);
                    }
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                flags->o_flag = 1;
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    // Если шаблон не был задан через флаг -e или -f, то предполагается, что он передан как аргумент
    if (!flags->e_flag && !flags->f_flag) {
        if (optind < argc) {
            flags->pattern = copy_string(argv[optind++]);
        } else {
            fprintf(stderr, "Ошибка: не указан шаблон для поиска.\n");
            return EXIT_FAILURE;
        }
    }

    // Компиляция регулярного выражения
    int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
    if (regcomp(&flags->regex, flags->pattern, regex_flags) != 0) {
        fprintf(stderr, "Ошибка компиляции регулярного выражения.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Проверка строки на совпадение с регулярным выражением
int MatchLine(const char *line, GrepFlags *flags) {
    int match = (regexec(&flags->regex, line, 0, NULL, 0) == 0);
    return flags->v_flag ? !match : match;
}

// Печать строки (с номерами, подсчёт совпадений и т.д.)
void PrintLine(const char *line, int line_number, GrepFlags *flags, int *match_count, const char *filename) {
    if (flags->c_flag) {
        (*match_count)++;
    } else if (flags->l_flag) {
        printf("%s\n", filename);
    } else {
        if (flags->n_flag) {
            printf("%d:", line_number);
        }
        if (flags->o_flag) {
            // Вывод только совпавших частей строки
            const char *start = line;
            regmatch_t pmatch;
            while (regexec(&flags->regex, start, 1, &pmatch, 0) == 0) {
                printf("%.*s\n", pmatch.rm_eo - pmatch.rm_so, start + pmatch.rm_so);
                start += pmatch.rm_eo;
            }
        } else {
            printf("%s", line);
        }
    }
}

// Обработка файла
void ProcessFile(FILE *file, GrepFlags *flags, const char *filename) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int line_number = 1;
    int match_count = 0;

    while ((nread = getline(&line, &len, file)) != -1) {
        if (MatchLine(line, flags)) {
            PrintLine(line, line_number, flags, &match_count, filename);
        }
        line_number++;
    }

    free(line);  // Освобождаем память
    if (flags->c_flag) {
        printf("%d\n", match_count);
    }
}

// Главная функция
int main(int argc, char *argv[]) {
    GrepFlags flags = {0};
    int return_flag = InitializeFlags(&flags, argc, argv);

    if (optind < argc) {
        // Работа с файлом
        FILE *file = fopen(argv[optind], "r");
        if (!file) {
            perror("fopen");
            return EXIT_FAILURE;
        }
        ProcessFile(file, &flags, argv[optind]);
        fclose(file);
    } else {
        // Работа с stdin
        ProcessFile(stdin, &flags, "stdin");
    }

    // Освобождение памяти
    free(flags.pattern);
    regfree(&flags.regex);
    return return_flag == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
