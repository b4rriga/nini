# Nini

Minimal INI-style configuration parser in C11. Static library. No dependencies. No runtime magic.

Named after the spanish term for NEET.

---

## Features

- `key=value` parsing
- Optional [section] support
- Integer, float, string, boolean inference
- Whitespace trimming
- Comment support (#, ;)
- Single-pass parser
- Deterministic behavior

---

## Installation

#### Build
```sh
make
```

#### Install
```sh
sudo make install
```

---

## Types

- `int`: 123
- `float`: 12.34
- `bool`: true / false / yes / no
- `string`: "text" or raw text

## Design

- C11 strict
- No POSIX extensions required
- No dynamic runtime dependencies
- No configuration framework overhead
- Intended for small UNIX tools and embedded systems

