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
cc main.c -lnini
```

## Usage

If an integer value had to be retrieved by key from an INI file `config.ini`, at section `numbers` and with key identifier `integer`, the library user would do as follows:

```c
#include <nini.h>
#include <stdio.h>

int main(void)
{
    Nini *cfg = nini_load("config.ini");
    if (!cfg) return 1;

    int err;
    printf("integer = %ld\n", nini_get_int(cfg, "numbers.integer", &err));
    printf("[status: %s]\n", nini_error(err));

    nini_free(cfg);
    return 0;
}
```

... getting the following output:

```text
integer = 42 [status: OPERATION OK]
```

Several INI files may be loaded simultaneously as each call to `nini_load()` returns a pointer to an opaque structure `Nini`.

In the example, a status integer (`ok`) is passed by reference, allowing the function to store the result state. This variable can be later rendered as human-readable string using `nini_error()`. This argument may be omitted by passing `NULL` in its place.

The numeric literal `42` is not limited to an integer representation. Since it is exactly representable as a floating-point value, Nini also allows it to be retrieved as a `double`. Likewise, the original textual representation can always be retrieved as a string. Nevertheless, given the library's design principles, it is safe to assume that any number that is not `0` or `1` (and by extension, `0.0` or `1.0`) will not be a valid boolean, thus the following output:

```text
integer        =         42 [status: OPERATION OK]
(float)integer =  42.000000 [status: OPERATION OK]
(bool)integer  =      false [status: CONVERSION NOT POSSIBLE]
```

... which results from the following tweak to the example entry point:

```c
int err;
printf("integer        = %10ld", nini_get_int(cfg, "numbers.integer", &err));
printf(" [status: %s]\n", nini_error(err));
printf("(float)integer = %10lf", nini_get_float(cfg, "numbers.integer", &err));
printf(" [status: %s]\n", nini_error(err));
printf("(bool)integer  = %10s", nini_get_bool(cfg, "numbers.integer", &err) ? "true" : "false");
printf(" [status: %s]\n", nini_error(err));
```

Obviously, indexing a section or a non-existent key results in an error:

```c
printf("numbers = %10ld", nini_get_int(cfg, "numbers", &err));
printf(" [status: %s]\n", nini_error(err));
printf("name    = %10s", nini_get_str(cfg, "name", &err));
printf(" [status: %s]\n", nini_error(err));
```

```text
numbers =          0 [status: KEY NOT FOUND]
name    =     (null) [status: KEY NOT FOUND]
```

This library supports two equivalent syntaxes for hierarchical keys. Subsections may be nested arbitrarily. The current implementation limits the total section path length to 255 characters, though this is not a fundamental limitation.

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

Using `nini_dump()` to print the parsed configuration yields the following output. It also illustrates that later assignments override earlier ones.

```text
numbers.integer
├── as int    : 1337
├── as float  : 1337.0
└── as string : "1337"
numbers.float
├── as float  : 2.71828
└── as string : "2.71828"
logic.bool
├── as bool   : false
└── as string : "off"
logic.bool2
├── as int    : 1
├── as float  : 1.0
├── as bool   : true
└── as string : "1.000"
string
└── as string : "howdy"
```

In this example, `string` is defined at the root level, meaning it does not belong to any section. With the `section.key` syntax, root-level keys are written without a section prefix. With the `[section]` syntax, they can be defined either before the first section is opened or after resetting the current section with `[]`, which returns the parser to the root level.

## Types

- **Integer**: `10`, `0b1010`, `012`, `0xA` (all evaluate to decimal `10`)
- **Float**: `12.34`
- **String**: `"text"` or `raw text`
- **Boolean**: `true`, `false`, `on`, `off`, `yes`, `no`

Hexadecimal digits, integer base prefixes and boolean literals are all case-insensitive.

If double quotes are intended to be part of the string, enclose the entire value in an additional pair of double quotes. The outer quotes are stripped during parsing, leaving the inner quotes as part of the string.

## Design

- C11 strict
- No POSIX extensions required
- No dynamic runtime dependencies
- No configuration framework overhead
- Intended for small UNIX tools and embedded systems

