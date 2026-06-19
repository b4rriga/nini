#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "nini.h"

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

static int is_bool(const char *s)
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

static void push(Nini_Config *c, Nini_Entry e)
{
    if (c->len == c->cap) {
        c->cap = c->cap ? c->cap * 2 : 16;
        c->data = realloc(c->data, c->cap * sizeof(Nini_Entry));
    }
    c->data[c->len++] = e;
}

static char current_section[64];

static void parse_line(Nini_Config *cfg, char *line)
{
    line = trim(line);

    if (*line == 0 || *line == '#' || *line == ';')
        return;

    if (*line == '[') {
        char *end = strchr(line, ']');
        if (!end) return;

        *end = 0;
        strncpy(current_section, line + 1, sizeof(current_section)-1);
        current_section[sizeof(current_section)-1] = 0;
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

        if (!strcmp(val, "true") || !strcmp(val, "1") || !strcmp(val, "yes"))
            e.v.i = 1;
        else
            e.v.i = 0;
    } else if (is_int(val)) {
        e.type = NINI_INT;
        e.v.i = strtol(val, NULL, 10);
    }
    else if (is_float(val)) {
        e.type = NINI_FLOAT;
        e.v.f = strtod(val, NULL);
    }
    else {
        e.type = NINI_STRING;
        e.v.s = unquote(val);
    }

    push(cfg, e);
}

bool load_ini(const char *path, Nini_Config *cfg)
{
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char line[512];
    current_section[0] = 0;

    while (fgets(line, sizeof(line), f)) {
        parse_line(cfg, line);
    }

    fclose(f);
    return true;
}

Nini_Entry *get(Nini_Config *cfg, const char *section, const char *key)
{
    for (size_t i = 0; i < cfg->len; i++) {
        if (strcmp(cfg->data[i].key, key) == 0 &&
            strcmp(cfg->data[i].section, section) == 0)
            return &cfg->data[i];
    }
    return NULL;
}
