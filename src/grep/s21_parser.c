#include "s21_grep.h"

int ParseArguments(GrepFlags *flags, int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'e':
      case 'f':
        HandleFlagEorF(flags, optarg,
                       opt);  // Передаем указатель на return_value
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
        fprintf(stderr, "Error: Invalid option: -%c\n", opt);
        flags->error_code = 1;
        break;
    }
  }

  if (!flags->error_code) {
    if (!flags->e_flag && !flags->f_flag && optind < argc) {
      AddPattern(&flags->combined_pattern, argv[optind++], flags);
    }
    ApplyFlagsPriority(flags);  // Убедитесь, что эта функция определена
    CompilePattern(flags);  // Убедитесь, что эта функция определена
  }
  return flags->error_code;
}

void HandleFlagEorF(GrepFlags *flags, const char *optarg, int opt) {
  if (!optarg) {
    fprintf(stderr, "Error: Option -%c requires an argument.\n", opt);
    flags->error_code = 1;
  }
  if (opt == 'e') {
    flags->e_flag = 1;
    AddPattern(&flags->combined_pattern, optarg, flags);
  } else if (opt == 'f') {
    flags->f_flag = 1;
    FILE *file = fopen(optarg, "r");
    if (file) {
      char line[4098];
      while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        AddPattern(&flags->combined_pattern, line, flags);
      }
      fclose(file);
    } else {
      fprintf(stderr, "Error: Could not open pattern file: %s\n", optarg);
      flags->error_code = 2;
    }
  }
}

void AddPattern(char **combined_pattern, const char *new_pattern,
                GrepFlags *flags) {
  size_t current_length = *combined_pattern ? strlen(*combined_pattern) : 0;
  size_t new_length =
      current_length + strlen(new_pattern) + (current_length > 0 ? 3 : 1);
  char *updated_pattern = realloc(*combined_pattern, new_length);
  if (updated_pattern) {
    *combined_pattern = updated_pattern;
    if (current_length > 0) {
      strcat(*combined_pattern, "\\|");
    } else {
      (*combined_pattern)[0] = '\0';
    }
    strcat(*combined_pattern, new_pattern);
  } else {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    flags->error_code = 3;
  }
}

void ApplyFlagsPriority(GrepFlags *flags) {
  if (flags->l_flag) flags->c_flag = flags->o_flag = flags->n_flag = 0;
  if (flags->c_flag) flags->o_flag = 0;
  if (flags->o_flag && flags->v_flag) flags->skip_processing = 1;
}