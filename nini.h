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

#ifndef NINI_H
#define NINI_H

#include <stdbool.h>

enum {
    NINI_OK,
    NINI_NOT_FOUND,
    NINI_BAD_CONVERSION
};

typedef struct Nini_Config Nini;

Nini *nini_load(const char *path);
void nini_dump(Nini *cfg);
void nini_free(Nini *cfg);

const char *nini_error(int error);

long nini_get_int(Nini *cfg, const char *key, int *err);
double nini_get_float(Nini *cfg, const char *key, int *err);
bool nini_get_bool(Nini *cfg, const char *key, int *err);
char *nini_get_str(Nini *cfg, const char *key, int *err);

#endif // NINI_H
