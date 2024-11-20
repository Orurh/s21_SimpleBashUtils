#include "s21_grep.h"

void InitializeFlags(GrepFlags *flags) {
  *flags = (GrepFlags){0};  // Инициализация всех полей в ноль
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

void CompilePattern(GrepFlags *flags, int *return_value) {
  if (!flags->pattern) {
    fprintf(stderr, "Error: No search pattern provided.\n");
    *return_value = 1;
    return;
  }

  int regex_flags = REG_EXTENDED | (flags->i_flag ? REG_ICASE : 0);
  if (regcomp(&flags->regex, flags->pattern, regex_flags) != 0) {
    fprintf(stderr, "Error: Invalid regular expression.\n");
    exit(EXIT_FAILURE);
  }
}

void ParseArguments(GrepFlags *flags, int argc, char *argv[],
                    int *return_value) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhs")) != -1) {
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
      default:
        *return_value = 1;
        fprintf(stderr, "Error: Unknown option -%c\n", opt);
        return;
    }
  }

  if (!flags->e_flag && optind < argc) {
    flags->pattern = argv[optind++];
    flags->pattern_count++;
  }

  CompilePattern(flags, return_value);
  flags->filename_flag = (argc - optind > 1);
}
