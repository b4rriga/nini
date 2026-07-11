# Nini

Minimal INI configuration parser for C11.

Nini parses INI files and stores their contents using the [`value`](https://github.com/b4rriga/b4se#value) module from [`b4se`](https://github.com/b4rriga/b4se), providing automatic type recognition and conversion while exposing a simple API for configuration retrieval.

Named after the Spanish term for NEET.

## Features

- `key=value` parsing
- Optional `[section]` support
- Automatic type inference through `b4se/value`
- Whitespace trimming
- Comment support (`#`, `;`)
- Single-pass parser
- Deterministic behavior

## Installation

Depends on `b4se` (stable release [26.07.11](https://github.com/b4rriga/b4se/releases/tag/26.07.11)).

### Build
```sh
make
```

### Install
```sh
sudo make install
```

### Linking
```sh
cc main.c -lnini -lb4se
```

## Usage

The following example retrieves the key `numbers.integer` from an INI file:

```c
#include <nini.h>
#include <stdio.h>

int main(void)
{
    Nini *cfg = nini_load("config.ini");
    if (!cfg) return 1;

    NiniStatus err;
    printf("integer = %ld", nini_get_int(cfg, "numbers.integer", &err));
    printf(" [status: %s]\n", nini_error(err));

    nini_free(cfg);
    return 0;
}
```

getting the following output:

```text
integer = 42 [status: OPERATION OK]
```

Each call to `nini_load()` returns an independent opaque `Nini` structure, allowing multiple configuration files to coexist simultaneously.

Lookup functions optionally report their result through a `NiniStatus` output parameter. Passing `NULL` suppresses status reporting.

Parsed values are stored by the `b4se/value` module, which provides automatic type inference and typed retrieval. See the `b4se` documentation for details.

## INI syntax

Nini supports two equivalent syntaxes for hierarchical keys. Subsections may be nested arbitrarily.

```ini
# section.key notation

numbers.integer = 42
numbers.float = 3.14159
logic.bool = true
logic.bool2 = no
string = "hello"

; [section] notation

[numbers]
integer = 1337
float = 2.71828

[logic]
bool = off
bool2 = 1.000

[]
string = howdy
```

Later assignments override earlier ones.

Root-level keys may be written without a section prefix or after resetting the current section with [].

## Types

- **Integer**: `10`, `0b1010`, `012`, `0xA` (all evaluate to decimal `10`)
- **Float**: `12.34`
- **String**: `"text"` or `raw text`
- **Boolean**: `true`, `false`, `on`, `off`, `yes`, `no`, `0`, `0.0`, `1`, `1.0`

Hexadecimal digits, integer base prefixes and boolean literals are all case-insensitive.

Quoted strings have their outer quotes removed during parsing. To preserve literal quotation marks, wrap the value in an additional pair of quotes.

## Design

- ISO C11
- No POSIX extensions
- No dynamic runtime dependencies
- Static library
- Small and portable
- Suitable for UNIX tools, embedded systems and freestanding projects with a C11 runtime
