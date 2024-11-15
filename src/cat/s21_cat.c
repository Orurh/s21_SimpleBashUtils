#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catFlags.h"

int main(int argc, char *argv[]) {
  int opt;
  int exit_status = 0;
  CatFlags flags = {0};

  struct option long_options[] = {
      {"number-nonblank", no_argument, &flags.number_nonblanck, 1},
      {"number", no_argument, &flags.number_all, 1},
      {"squeeze-blank", no_argument, &flags.squeeze_blank, 1},
      {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "bnsevtET", long_options, NULL)) !=
         -1) {
    if (opt == 'b')
      flags.number_nonblanck = 1;
    else if (opt == 'n')
      flags.number_all = 1;
    else if (opt == 's')
      flags.squeeze_blank = 1;
    else if (opt == 'e')
      flags.show_ends = flags.show_hiddensimv = 1;
    else if (opt == 't')
      flags.show_tabs = flags.show_hiddensimv = 1;
    else if (opt == 'v')
      flags.show_hiddensimv = 1;
    else if (opt == 'E')
      flags.show_ends = 1;
    else if (opt == 'T')
      flags.show_tabs = 1;
    else if (opt == '?')
      exit_status = 1;
  }

  if (!exit_status) {
    FILE *fp = NULL;
    int lineNumber = 1, prevEmpty = 0;
    char *line = NULL;
    size_t lineSize = 0;

    if (optind >= argc) fp = stdin;

    for (int i = optind; i < argc || (optind >= argc && fp); i++) {
      if (i < argc) {
        if (!(fp = fopen(argv[i], "r"))) {
          perror(argv[i]);
          continue;
        }
      }

      while (getline(&line, &lineSize, fp) != -1) {
        PrintLineWithOptions(line, &lineNumber, &flags, &prevEmpty);
      }

      if (fp && fp != stdin) fclose(fp);
      fp = NULL;
    }

    free(line);
  }
  return exit_status;
}