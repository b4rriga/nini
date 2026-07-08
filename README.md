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

If an integer value must be retrieved by key from an INI file `config.ini`, the library user would do as follows:

```c
#include <nini.h>
#include <stdio.h>

int main(void)
{
    Nini *cfg = nini_load("config.ini");
    if (!cfg) return 1;

    int err;
    printf("integer = %ld\n", nini_get_int(cfg, "numbers", "integer", &err));
    printf("exit code: %s\n", nini_error(err));

    nini_free(cfg);
    return 0;
}
```

Several INI files may be loaded simultaneously as each call to `nini_load()` returns a pointer to an opaque structure `Nini`.

In the example, a status integer is passed by reference, allowing the function to store the result state. This variable can be later rendered as human-readable string using `nini_error()`. This argument may be omitted by passing `NULL` in its place.

This library supports two equivalent syntaxes for hierarchical keys. Subsections may be nested arbitrarily; the current implementation limits the total section path length to 255 characters, though this is not a fundamental limitation.

```ini
# section.key notation

numbers.integer = 42
numbers.float = 3.14159
misc.bool = true
misc.bool2 = no
string = "hello"

; [section] notation

[numbers]
integer = 1337
float = 2.71828

[misc]
bool = false
bool2 = yes

[]
string = howdy
```

Using `nini_dump()` to print the parsed configuration yields the following output. It also illustrates that later assignments override earlier ones.

```text
numbers.integer = 1337
numbers.float = 2.718280
misc.bool = false
misc.bool2 = true
string = "howdy"
```

In the example INI file, `string` is a key defined at the root of the file, that is, it is not under any section. If using the `section.key` notation, it can be defined as a standalone key by simply omitting a section name followed by the dot separator. If using the `[section]` notation, it can be defined either by assigning a value before any section is opened, or alternatively employing the `[]` trick, which reset the current section back into the root.

In this example, `string` is defined at the root level, meaning it does not belong to any section. With the `section.key` syntax, root-level keys are written without a section prefix. With the `[section]` syntax, they can be defined either before the first section is opened or after resetting the current section with `[]`, which returns the parser to the root level.

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

