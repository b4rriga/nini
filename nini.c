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

typedef enum {
    NINI_INT,
    NINI_FLOAT,
    NINI_STR,
    NINI_BOOL
} Nini_Type;

typedef struct {
    union {
        long i;
        double f;
        bool b;
        char *s;
    } v;
    Nini_Type type;
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

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') s++;

    char *end = s + strlen(s);
    while (end > s && (
           end[-1] == ' ' || end[-1] == '\t' ||
           end[-1] == '\n' || end[-1] == '\r'))
        *--end = 0;

    return s;
}

// TODO: support binary, octal and hexadecimal
static bool is_int(const char *s)
{
    if (!*s) return false;
    char *e;
    strtol(s, &e, 10);
    return *e == 0;
}

static bool is_float(const char *s)
{
    if (!strchr(s, '.')) return false;
    char *e;
    strtod(s, &e);
    return *e == 0;
}

// TODO: support (-inf, 0] as false and [1, inf) as true
// TODO: be case-insensitive
static bool is_bool(const char *s)
{
    return (!strcmp(s, "true")  ||
            !strcmp(s, "false") ||
            !strcmp(s, "yes")   ||
            !strcmp(s, "no"));
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
        if (old->type == NINI_STR)
            free(old->v.s);

        *old = e;
        return;
    }

    if (cfg->len == cfg->cap) {
        cfg->cap = cfg->cap ? cfg->cap * 2 : 16;
        cfg->data = realloc(cfg->data, cfg->cap * sizeof(Nini_Entry));
    }

    cfg->data[cfg->len++] = e;
}

static char section[256];

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

    Nini_Entry e;
    if (*section == '\0')
        e.key = xstrdup(key);
    else {
        size_t len = strlen(section) + 1 + strlen(key) + 1;
        e.key = malloc(len);
        snprintf(e.key, len, "%s.%s", section, key);
    }

    // TODO: support multiple representation of the same value
    if (is_bool(val)) {
        e.type = NINI_BOOL;

        if (!strcmp(val, "true") || !strcmp(val, "yes"))
            e.v.b = true;
        else
            e.v.b = false;
    } else if (is_int(val)) {
        e.type = NINI_INT;
        e.v.i = strtol(val, NULL, 10);
    }
    else if (is_float(val)) {
        e.type = NINI_FLOAT;
        e.v.f = strtod(val, NULL);
    }
    else {
        e.type = NINI_STR;
        e.v.s = unquote(val);
    }

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
        const Nini_Entry entry = cfg->data[i];
        printf("%s = ", entry.key);

        switch (entry.type) {
        case NINI_INT:
            printf("%ld\n", entry.v.i);
            break;
        case NINI_FLOAT:
            printf("%lf\n", entry.v.f);
            break;
        case NINI_BOOL:
            printf("%s\n", entry.v.b ? "true" : "false");
            break;
        case NINI_STR:
            printf("\"%s\"\n", entry.v.s);
            break;
        }
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
    case NINI_OK:            return "OK";
    case NINI_NOT_FOUND:     return "NOT FOUND";
    case NINI_TYPE_MISMATCH: return "TYPE MISMATCH";
    default:                 return "UNKNOWN ERROR";
    }
}

#define NINI_FAIL(x) do { if (err) *err = (x); return 0; } while (0)

long nini_get_int(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_INT) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.i;
}

double nini_get_float(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_FLOAT) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.f;
}

bool nini_get_bool(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_BOOL) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.b;
}

char *nini_get_str(Nini *cfg, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_STR) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.s;
}

#undef NINI_FAIL
