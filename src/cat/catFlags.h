#ifndef CAT_FLAGS_H
#define CAT_FLAGS_H

#include <stdio.h>


typedef struct {
    int number_all;
    int number_nonblanck;
    int squeeze_blank;
    int show_ends;
    int show_tabs;
    int show_hiddensimv;
} CatFlags;


void ProcessFile(FILE *fp, const CatFlags *flags, int *index, int *previous);

#endif