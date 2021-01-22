# C File Wrapper for C++

## Overview

A header-only C++ library intended to provide a safe and easy to use wrapper class for C file streams. The class contains static and member functions to access the entire `<cstdio>` [header](https://en.cppreference.com/w/cpp/header/cstdio).

Not to be confused with [MFC CFile](https://docs.microsoft.com/en-us/cpp/mfc/reference/cfile-class).

## Features
- Strong ownership semantics for `std::FILE *` (copying is disallowed)
- Automatic management of file resources using RAII techniques
- Same class size and alignment as `std::FILE *`
- Follows the zero-cost abstraction principle (don't pay for what you don't use)
- Debug runtime assertions for common misuses
- Easy access to the underlying `std::FILE *` for compatibility with existing code
- Specifying the file access mode with an enum
- All functions are `noexcept`

## Project Requirements
C++14 language version.

## License
Licensed under [MIT](LICENSE).
