#include "s21_grep.h"
#include <getopt.h>

int main(int argc, char *argv[]) {
  int return_value = 0;
  GrepFlags flags;
  InitializeFlags(&flags);

  ParseArguments(&flags, argc, argv, &return_value);
  if (return_value != 0) {
    return EXIT_FAILURE;
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: No files provided.\n");
    FreeResources(&flags);
    return EXIT_FAILURE;
  }

  for (int i = optind; i < argc; ++i) {
    const char *filename = argv[i];
    FILE *file = OpenFile(filename, &flags);
    if (file) {
      ProcessFile(file, &flags, filename);
      fclose(file);
    }
  }

  FreeResources(&flags);
  return return_value;
}