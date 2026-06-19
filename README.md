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

