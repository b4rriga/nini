// SPDX-License-Identifier: LGPL-3.0-or-later
// This file is part of Nini.

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nini.h"

static char section[256]; // TODO: convert to dynamic array;
                          //       leverage b4se/dynarr.h

// Copy-pasted from b4se/value/utils.c
// TODO: get from b4se/strplus.h
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

    *eq = '\0';

    char *key = trim(line);
    char *val = trim(eq + 1);

    if (*section == '\0')
        b4_value_store(cfg, key, val);
    else {
        size_t len = strlen(section) + 1 + strlen(key) + 1;
        char *fullkey = malloc(len);
        if (!fullkey) return;

        snprintf(fullkey, len, "%s.%s", section, key);
        b4_value_store(cfg, key, val);
        free(fullkey);
    }
}

Nini *nini_load(const char *path)
{
    Nini *cfg = b4_value_new();
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
