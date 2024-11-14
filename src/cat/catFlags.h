#ifndef CAT_FLAGS_H
#define CAT_FLAGS_H



typedef struct {
    int numberAll;
    int numberNonblank;
    int squeezeBlank;
    int showEnds;
    int showTabs;
    int showHiddensimv;
} CatFlags;


void PrintLineWithOptions(const char *line, int *lineNumber, CatFlags *flags, int *prevEmpty);

#endif