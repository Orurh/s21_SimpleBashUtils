#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для хранения флагов
typedef struct {
  int e_flag, i_flag, v_flag, c_flag, l_flag, n_flag, h_flag, s_flag, f_flag,
      o_flag, skip_processing;
  char *combined_pattern;
  regex_t regex;
  int error_code;
} GrepFlags;

int ParseArguments(GrepFlags *flags, int argc, char *argv[]);
void HandleFlagEorF(GrepFlags *flags, const char *optarg, int opt);
void AddPattern(char **combined_pattern, const char *new_pattern,
                GrepFlags *flags);
void CompilePattern(GrepFlags *flags);
void ApplyFlagsPriority(GrepFlags *flags);
void FreeResources(GrepFlags *flags);
void CompilePattern(GrepFlags *flags);
int CheckMatch(const char *line, const regex_t *regex, int invert_match);
void ProcessFile(FILE *file, const char *filename, const GrepFlags *flags,
                 int total_files);
int ProcessLine(const char *line, int line_number, const GrepFlags *flags,
                const char *filename, int total_files, int *match_count,
                int *file_matched);
void FreeResources(GrepFlags *flags);
FILE *OpenFile(const char *filename, const GrepFlags *flags);
void PrintMatchDetails(const char *line, int line_number,
                       const GrepFlags *flags, const char *filename,
                       int total_files);

#endif  // S21_GREP_H
