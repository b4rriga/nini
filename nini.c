//
// This file is part of Nini.
//
// Nini is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// Nini is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Nini. If not, see <https://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "nini.h"

#define NINI_INT   (1u << 0)
#define NINI_FLOAT (1u << 1)
#define NINI_BOOL  (1u << 2)
#define NINI_STR   0

static char section[256];

typedef struct {
    struct {
        long i;
        double f;
        bool b;
        char *s;
    } value;
    uint8_t types;
    char *key;
} Nini_Entry;

struct Nini_Config {
    Nini_Entry *data;
    int len, cap;
};

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static bool stricmp(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a++;
        char cb = *b++;

        if ('A' <= ca && ca <= 'Z')
            ca |= 0x20;
        if ('A' <= cb && cb <= 'Z')
            cb |= 0x20;

        if (ca != cb)
            return false;
    }

    return *a == *b;
}

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') s++;

    char *end = s + strlen(s);
    while (end > s && (
           end[-1] == ' '  || end[-1] == '\t' ||
           end[-1] == '\n' || end[-1] == '\r'))
        *--end = 0;

    return s;
}

static char *unquote(const char *s)
{
    size_t n = strlen(s);

    if (n >= 2 && s[0] == '"' && s[n - 1] == '"') {
        char *r = malloc(n - 1);
        memcpy(r, s + 1, n - 2);
        r[n - 2] = 0;
        return r;
    }

    return xstrdup(s);
}

// TODO: accept #AABBCC and #AABBCCDD as number for use as color
static bool is_int(const char *s)
{
    if (!*s) return false;
    char *e;

    if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
        strtol(s + 2, &e, 2);
        if (*e == 0) return true;
    } else {
        strtol(s, &e, 0);
        if (*e == 0) return true;
    }

    double d  = strtod(s, &e);
    if (*e != 0) return false;

    return d == (double)(long)d;
}

// TODO: support cientific notation
static bool is_float(const char *s)
{
    if (!*s) return false;
    char *e;

    if (strchr(s, '.')) {
        strtod(s, &e);
        if (*e == 0) return true;
    }

    return is_int(s);
}

static bool is_bool(const char *s)
{
    if (stricmp(s, "true")  ||
        stricmp(s, "false") ||
        stricmp(s, "on")    ||
        stricmp(s, "off")   ||
        stricmp(s, "yes")   ||
        stricmp(s, "no"))
        return true;

    char *e;
    double d = strtod(s, &e);
    if (*e != 0) return false;

    return d == 0.0 || d == 1.0;
}

static void store_int(Nini_Entry *e, const char *val)
{
    e->types |= NINI_INT;

    if (val[0] == '0' && (val[1] == 'b' || val[1] == 'B'))
        e->value.i = strtol(val + 2, NULL, 2);
    else
        e->value.i = strtol(val, NULL, 0);
}

static void store_float(Nini_Entry *e, const char *val)
{
    e->types |= NINI_FLOAT;

    e->value.f = strtod(val, NULL);
}

static void store_bool(Nini_Entry *e, const char *val)
{
    e->types |= NINI_BOOL;

    if (stricmp(val, "true") ||
        stricmp(val, "on")   ||
        stricmp(val, "yes")) {
        e->value.b = true;
        return;
    }

    if (stricmp(val, "false") ||
        stricmp(val, "off")   ||
        stricmp(val, "no")) {
        e->value.b = false;
        return;
    }

    e->value.b = (strtod(val, NULL) == 1.0);
}

static Nini_Entry *get(Nini *cfg, const char *key)
{
    for (int i = 0; i < cfg->len; i++)
        if (strcmp(cfg->data[i].key, key) == 0)
            return &cfg->data[i];

    return NULL;
}

static void push(Nini *cfg, Nini_Entry e)
{
    Nini_Entry *old = get(cfg, e.key);

    if (old) {
        free(old->key);
        free(old->value.s);

        *old = e;
        return;
    }

    if (cfg->len == cfg->cap) {
        cfg->cap = cfg->cap ? cfg->cap * 2 : 16;
        cfg->data = realloc(cfg->data, cfg->cap * sizeof(Nini_Entry));
    }

    cfg->data[cfg->len++] = e;
}

static void parse_line(Nini *cfg, char *line)
{
    line = trim(line);

    if (*line == 0 || *line == '#' || *line == ';') return;

    if (*line == '[') {
        char *end = strchr(line, ']');
        if (!end) return;

        *end = 0;
        strncpy(section, line + 1, sizeof(section) - 1);
        section[sizeof(section) - 1] = 0;
        return;
    }

    char *eq = strchr(line, '=');
    if (!eq) return;

    *eq = 0;

    char *key = trim(line);
    char *val = trim(eq + 1);

    Nini_Entry e = {0};

    if (*section == '\0')
        e.key = xstrdup(key);
    else {
        size_t len = strlen(section) + 1 + strlen(key) + 1;
        e.key = malloc(len);
        snprintf(e.key, len, "%s.%s", section, key);
    }

    if (is_int(val))   store_int(&e, val);
    if (is_float(val)) store_float(&e, val);
    if (is_bool(val))  store_bool(&e, val);
    e.value.s = unquote(val);

    push(cfg, e);
}

Nini *nini_load(const char *path)
{
    Nini *cfg = calloc(1, sizeof(*cfg));
    if (!cfg) return NULL;

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    char line[512];
    section[0] = '\0';

    while (fgets(line, sizeof(line), f))
        parse_line(cfg, line);

    fclose(f);
    return cfg;
}

void nini_dump(Nini *cfg)
{
    for (int i = 0; i < cfg->len; i++) {
        const Nini_Entry e = cfg->data[i];
        fprintf(stderr, "%s\n", e.key);

        if (e.types & NINI_INT)
            fprintf(stderr, "├── as int    : %ld\n", e.value.i);
        if (e.types & NINI_FLOAT) {
            fprintf(stderr, "├── as float  : %g", e.value.f);

            if (e.types & NINI_INT)
                fprintf(stderr, ".0");
            fprintf(stderr, "\n");
        }
        if (e.types & NINI_BOOL)
            fprintf(stderr, "├── as bool   : %s\n", e.value.b ? "true" : "false");

        fprintf(stderr, "└── as string : \"%s\"\n", e.value.s);
    }
}

void nini_free(Nini *cfg)
{
    free(cfg->data);
    free(cfg);
}

const char *nini_error(int error)
{
    switch (error) {
    case NINI_OK:             return "OPERATION OK";
    case NINI_NOT_FOUND:      return "KEY NOT FOUND";
    case NINI_BAD_CONVERSION: return "CONVERSION NOT POSSIBLE";
    default:                  return "UNKNOWN ERROR";
    }
}

#define NINI_FAIL(x) do { if (err) *err = (x); return 0; } while (0)

static Nini_Entry *get_as(Nini *cfg, const char *key, uint8_t type, int *err)
{
    Nini_Entry *e = get(cfg, key);

    if (!e) {
        if (err) *err = NINI_NOT_FOUND;
        return NULL;
    }

    if (!(e->types & type)) {
        if (err) *err = NINI_BAD_CONVERSION;
        return NULL;
    }

    if (err) *err = NINI_OK;

    return e;
}

long nini_get_int(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get_as(cfg, key, NINI_INT, err);
    return e ? e->value.i : 0;
}

double nini_get_float(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get_as(cfg, key, NINI_FLOAT, err);
    return e ? e->value.f : 0.0;
}

bool nini_get_bool(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get_as(cfg, key, NINI_BOOL, err);
    return e ? e->value.b : false;
}

char *nini_get_str(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get_as(cfg, key, NINI_STR, err);
    return e ? e->value.s : NULL;
}

#undef NINI_FAIL
