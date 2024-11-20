#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Структура для хранения флагов
typedef struct {
  bool e_flag;  // Использование регулярного выражения
  bool i_flag;  // Игнорировать регистр
  bool v_flag;  // Инвертировать результат поиска
  bool c_flag;  // Подсчитать количество совпадений
  bool l_flag;  // Показать имя файлов с совпадениями
  bool n_flag;  // Показать номер строки
  bool h_flag;  // Не показывать имена файлов
  bool s_flag;  // Молчание, игнорировать ошибки при открытии файлов
  bool f_flag;  // Чтение паттернов из файла
  bool o_flag;  // Выводить только совпавшие части строки
  char *pattern;
  char *pattern_file;
  int pattern_count;
  int filename_flag;
  regex_t *regexes;  // Массив регулярных выражений
  int regex_count;   // Количество паттернов
} GrepFlags;


#endif // S21_GREP_H
