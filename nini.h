#ifndef NINI_H
#define NINI_H

typedef enum {
    NINI_INT,
    NINI_FLOAT,
    NINI_STRING,
    NINI_BOOL
} Nini_Type;

typedef struct {
    char *section;
    char *key;
    Nini_Type type;
    union {
        long i;
        double f;
        char *s;
    } v;
} Nini_Entry;

typedef struct {
    Nini_Entry *data;
    size_t len;
    size_t cap;
} Nini_Config;

#endif // NINI_H
