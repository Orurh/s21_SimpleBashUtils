#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для хранения флагов
typedef struct {
    int e_flag;
    int i_flag;
    int v_flag;
    int c_flag;
    int l_flag;
    int n_flag;
    int h_flag;
    int s_flag;
    regex_t regex;
    char *pattern;
    int pattern_count;
    int filename_flag;
} GrepFlags;

// Прототипы функций
void InitializeFlags(GrepFlags *flags);
void ParseArguments(GrepFlags *flags, int argc, char *argv[], int *return_value);
void HandleFlagE(GrepFlags *flags, char *optarg, int *return_value);
void CompilePattern(GrepFlags *flags, int *return_value);

FILE *OpenFile(const char *filename, GrepFlags *flags);
void ProcessFile(FILE *file, GrepFlags *flags, const char *filename);

void ProcessLine(const char *line, int line_number, GrepFlags *flags, int *match_count, const char *filename);

void FreeResources(GrepFlags *flags);

#endif // S21_GREP_H
