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
    char *section;
    char *key;
    Nini_Type type;
    union {
        long i;
        double f;
        bool b;
        char *s;
    } v;
} Nini_Entry;

struct Nini_Config {
    Nini_Entry *data;
    size_t len;
    size_t cap;
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
    while (end > s &&
          (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r'))
        *--end = 0;

    return s;
}

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

    if (n >= 2 && s[0] == '"' && s[n-1] == '"') {
        char *r = malloc(n - 1);
        memcpy(r, s + 1, n - 2);
        r[n - 2] = 0;
        return r;
    }

    return xstrdup(s);
}

static void push(Nini *c, Nini_Entry e)
{
    if (c->len == c->cap) {
        c->cap = c->cap ? c->cap * 2 : 16;
        c->data = realloc(c->data, c->cap * sizeof(Nini_Entry));
    }
    c->data[c->len++] = e;
}

static char current_section[64];

static void parse_line(Nini *cfg, char *line)
{
    line = trim(line);

    if (*line == 0 || *line == '#' || *line == ';')
        return;

    if (*line == '[') {
        char *end = strchr(line, ']');
        if (!end) return;

        *end = 0;
        strncpy(current_section, line + 1, sizeof(current_section) - 1);
        current_section[sizeof(current_section) - 1] = 0;
        return;
    }

    char *eq = strchr(line, '=');
    if (!eq) return;

    *eq = 0;

    char *key = trim(line);
    char *val = trim(eq + 1);

    Nini_Entry e;
    e.section = xstrdup(current_section);
    e.key = xstrdup(key);

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

static Nini_Entry *get(Nini *cfg, const char *section, const char *key)
{
    for (size_t i = 0; i < cfg->len; i++) {
        if (strcmp(cfg->data[i].key, key) == 0 &&
            strcmp(cfg->data[i].section, section) == 0)
            return &cfg->data[i];
    }

    return NULL;
}

Nini *nini_load(const char *path)
{
    Nini *cfg = calloc(1, sizeof(*cfg));
    if (!cfg) return NULL;

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    char line[512];
    current_section[0] = 0;

    while (fgets(line, sizeof(line), f)) {
        parse_line(cfg, line);
    }

    fclose(f);
    return cfg;
}

void nini_free(Nini *cfg)
{
    free(cfg->data);
    free(cfg);
}

const char *nini_error(int error)
{
    switch (error) {
    case NINI_OK:
        return "OK";
        break;
    case NINI_NOT_FOUND:
        return "NOT FOUND";
        break;
    case NINI_TYPE_MISMATCH:
        return "TYPE MISMATCH";
        break;
    default:
        return "UNKNOWN ERROR";
    }
}

#define NINI_FAIL(x) do { if (err) *err = (x); return 0; } while (0)

long nini_get_int(Nini *cfg, const char *section, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, section, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_INT) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.i;
}

double nini_get_float(Nini *cfg, const char *section, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, section, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_FLOAT) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.f;
}

bool nini_get_bool(Nini *cfg, const char *section, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, section, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_BOOL) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.b;
}

char *nini_get_str(Nini *cfg, const char *section, const char *key, int *err)
{
    Nini_Entry *e = get(cfg, section, key);

    if (!e) NINI_FAIL(NINI_NOT_FOUND);
    if (e->type != NINI_STR) NINI_FAIL(NINI_TYPE_MISMATCH);

    if (err) *err = NINI_OK;
    return e->v.s;
}

#undef NINI_FAIL
