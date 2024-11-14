#include <ctype.h>
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_file(FILE *fp, const char *pattern, int case_insensitive,
                  int invert_match, int count_only, int print_line_numbers) {
  char *line = NULL;
  size_t len = 0;
  size_t line_number = 0;
  int match_found = 0;

  while (getline(&line, &len, fp) != -1) {
    line_number++;
    regex_t regex;
    int reti;

    if (case_insensitive) {
      reti = regcomp(&regex, pattern, REG_ICASE);
    } else {
      reti = regcomp(&regex, pattern, 0);
    }

    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      exit(1);
    }

    reti = regexec(&regex, line, 0, NULL, 0);
    int is_match = (reti == 0);

    if (invert_match) {
      is_match = !is_match;
    }

    if (is_match) {
      match_found++;
      if (count_only) {

        continue;
      }
      if (print_line_numbers) {
        printf("%zu: ", line_number);
      }
      printf("%s", line);
    }
    regfree(&regex);
  }

  if (count_only) {
    printf("%d\n", match_found);
  }

  free(line);
}

int main(int argc, char *argv[]) {
  char *keys = NULL;
  int case_insensitive = 0;
  int invert_match = 0;
  int count_only = 0;
  int print_line_numbers = 0;
  int option;

  while ((option = getopt(argc, argv, "e:vlcn")) != -1) {
    switch (option) {
    case 'e':
      keys = optarg;
      break;
    case 'i':
      case_insensitive = 1;
      break;
    case 'v':
      invert_match = 1;
      break;
    case 'c':
      count_only = 1;
      break;
    case 'l':

      break;
    case 'n':
      print_line_numbers = 1;
      break;
    case '?':
    default:
      fprintf(stderr, "Usage: %s [-e pattern] [-i] [-v] [-c] [-n] filename\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Expected filename after options\n");
    exit(EXIT_FAILURE);
  }

  FILE *fp = fopen(argv[optind], "r");
  if (fp == NULL) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  if (keys == NULL) {
    fprintf(stderr, "Regular expression (-e) must be specified\n");
    fclose(fp);
    return EXIT_FAILURE;
  }

  process_file(fp, keys, case_insensitive, invert_match, count_only,
               print_line_numbers);

  fclose(fp);
  return EXIT_SUCCESS;
}
