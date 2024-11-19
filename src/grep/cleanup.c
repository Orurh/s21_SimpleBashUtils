#include "s21_grep.h"

void FreeResources(GrepFlags *flags) { regfree(&flags->regex); }
