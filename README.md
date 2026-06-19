# Nini

Minimal INI-style configuration parser in C11. Static library. No dependencies. No runtime magic.

Named after the Spanish term for NEET.

## Features

- `key=value` parsing
- Optional `[section]` support
- Integer, float, string, boolean inference
- Whitespace trimming
- Comment support (`#`, `;`)
- Single-pass parser
- Deterministic behavior

## Installation

#### Build
```sh
make
```

#### Install
```sh
sudo make install
```

## Usage

If an integer value must be retrieved by key from an INI file `config.ini`, the library user would do as follows:

```c
#include <nini.h>
#include <stdio.h>

int main(void)
{
    Nini *cfg = nini_load("config.ini");
    if (!cfg) return 1;

    int ok;
    printf("integer = %ld\n", nini_get_int(cfg, "numbers", "integer", &ok));
    printf("exit code: %s\n", nini_error(ok));

    nini_free(cfg);
    return 0;
}
```

Several INI files may be loaded simultaneously as each call to `nini_load()` returns a pointer to an opaque `struct Nini`.

In the example, a status integer is passed by reference, allowing the function to store the result state. This variable can be later rendered as human-readable string using `nini_error()`. This argument may be omitted by passing `NULL` in its place.

## Types

- Integer: `123`
- Float: `12.34`
- String: `"text"` or `raw text`
- Boolean: `true`, `false`, `yes`, `no`

## Design

- C11 strict
- No POSIX extensions required
- No dynamic runtime dependencies
- No configuration framework overhead
- Intended for small UNIX tools and embedded systems

