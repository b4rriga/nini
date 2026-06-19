#ifndef NINI_H
#define NINI_H

#include <stdbool.h>

enum {
    NINI_OK,
    NINI_NOT_FOUND,
    NINI_TYPE_MISMATCH
};

typedef struct Nini_Config Nini;

Nini *nini_load(const char *path);
void nini_free(Nini *cfg);

const char *nini_error(int error);

long   nini_get_int(Nini *cfg, const char *section, const char *key, int *ok);
double nini_get_float(Nini *cfg, const char *section, const char *key, int *ok);
bool   nini_get_bool(Nini *cfg, const char *section, const char *key, int *ok);
char  *nini_get_str(Nini *cfg, const char *section, const char *key, int *ok);

#endif // NINI_H
