#ifndef CAT_FLAGS_H
#define CAT_FLAGS_H



typedef struct {
    int number_all;
    int number_nonblanck;
    int squeeze_blank;
    int show_ends;
    int show_tabs;
    int show_hiddensimv;
} CatFlags;


void PrintLineWithOptions(const char *line, int *lineNumber, CatFlags *flags, int *prevEmpty);

#endif