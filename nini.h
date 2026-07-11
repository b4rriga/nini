// SPDX-License-Identifier: LGPL-3.0-or-later
// This file is part of Nini.

#ifndef NINI_H
#define NINI_H

#include <b4se/value.h>
#include <stdbool.h>
#include <stdint.h>

typedef B4Values Nini;
typedef B4ValueStatus NiniStatus;

Nini *nini_load(const char *path);

// -------------------------
//   b4se/value.h wrappers
// -------------------------

static inline void nini_dump(Nini *cfg)
{
    b4val_dump(cfg);
}

static inline void nini_free(Nini *cfg)
{
    b4val_free(cfg);
}

static inline const char *nini_error(NiniStatus err)
{
    return b4val_error(err);
}

static inline char *nini_get_str(Nini *cfg, const char *key, NiniStatus *err)
{
    return b4val_get_str(cfg, key, err);
}

static inline bool nini_get_bool(Nini *cfg, const char *key, NiniStatus *err)
{
    return b4val_get_bool(cfg, key, err);
}

static inline int64_t nini_get_int(Nini *cfg, const char *key, NiniStatus *err)
{
    return b4val_get_int(cfg, key, err);
}

static inline double nini_get_float(Nini *cfg, const char *key, NiniStatus *err)
{
    return b4val_get_float(cfg, key, err);
}

// TODO: graceful set functions (append if already set in file)
// TODO: forceful set functions (overwrite and prune duplicate instances)

#endif // NINI_H
