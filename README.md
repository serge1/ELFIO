<!--- # <img src="res/20231119165006-100.png" width="100"> ELFIO -->
# ![ELFIO Logo](doc/images/res/20231119165006-100.png "ELFIO") ![ELFIO Title](doc/images/res/title.png "ELFIO")

![C/C++ CI](https://github.com/serge1/ELFIO/workflows/C/C++%20CI/badge.svg)
![CodeQL](https://github.com/serge1/ELFIO/workflows/CodeQL/badge.svg)
[![Documentation](https://img.shields.io/badge/doc-download-brightgreen)](http://elfio.sourceforge.net/elfio.pdf)
[![License](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://github.com/serge1/ELFIO/blob/master/COPYING)

---

## Overview

**ELFIO** and **ARIO** are robust, header-only C++ libraries designed to make binary file and archive manipulation effortless, efficient, and portable. Whether you are building compilers, linkers, binary analysis tools, or custom build systems, these libraries provide the foundation you need for working with ELF files and UNIX archives.

---

## ELFIO: ELF Object and Executable File Reader/Writer

**ELFIO** is a lightweight, header-only C++ library for reading and generating ELF (Executable and Linkable Format) binary files. It is completely standalone, requiring no external dependencies, and integrates seamlessly into any C++ project. Built to ISO C++ standards, ELFIO ensures compatibility across a wide range of architectures and compilers.

**Key Features:**

- **Header-only:** Just include the header files—no need to build or link external libraries.
- **No dependencies:** Pure C++ implementation.
- **Cross-platform:** Works on Windows, Linux, and macOS.
- **Comprehensive ELF support:** Read, create, and modify ELF files, including sections, segments, and symbols.
- **Easy integration:** Designed for both small utilities and large-scale applications.
- **Actively maintained:** Trusted by open-source and commercial projects worldwide.

> 📖 Comprehensive documentation is available in the [ELFIO - Tutorial and User Manual (PDF)](http://elfio.sourceforge.net/elfio.pdf).

---

## ARIO: Advanced Archive Input/Output Library

**ARIO** is a modern, high-performance, header-only C++ library for reading, creating, and modifying UNIX `ar` archive files (commonly used for static libraries). ARIO is designed to work seamlessly with ELFIO, providing a unified and intuitive interface for archive manipulation and binary data management.

**Why Choose ARIO?**

- **Header-only:** Effortless integration—just include `ario.hpp` in your project.
- **Zero dependencies:** No need for external libraries or build steps.
- **Universal access:** Read and write to files, memory, and custom streams.
- **Cross-platform:** Consistent behavior on Windows, Linux, and macOS.
- **Optimized for performance:** Minimal overhead for high-throughput applications.
- **Seamless ELFIO integration:** Easily combine ELF and archive operations in your toolchain.
- **Intuitive API:** Designed for productivity and ease of use.

**Typical Use Cases:**

- Building and modifying static libraries (`.a` files)
- Extracting or replacing object files within archives
- Analyzing and manipulating symbol tables in archives
- Custom build tools and binary utilities
- Automated toolchains and CI/CD systems

---

## Getting Started

1. **Add the header files** to your project:
   - For ELFIO: `#include <elfio/elfio.hpp>`
   - For ARIO: `#include <ario/ario.hpp>`

2. **No build steps required:** Both libraries are header-only.

3. **Example: Reading an ELF file**

   ```cpp
   #include <elfio/elfio.hpp>
   ELFIO::elfio reader;
   if (reader.load("my_binary.elf")) {
       // Access ELF sections, segments, symbols, etc.
   }
   ```

4. **Example: Reading an archive file**

   ```cpp
   #include <ario/ario.hpp>
   ARIO::ario archive;
   if (archive.load("libmylib.a").ok()) {
       for (const auto& member : archive.members) {
           std::cout << "Member: " << member.name << std::endl;
       }
   }
   ```

---

## Who Uses ELFIO & ARIO?

- Open-source projects
- Commercial toolchains
- Academic research
- Embedded systems
- Binary analysis and reverse engineering tools

---

## Contributing

Contributions, bug reports, and feature requests are welcome! Please open an issue or submit a pull request on [GitHub](https://github.com/serge1/ELFIO).

---

## License

This project is licensed under the [MIT License](https://github.com/serge1/ELFIO/blob/master/COPYING).

---

## Resources

- [ELFIO Documentation (PDF)](http://elfio.sourceforge.net/elfio.pdf)
- [ELFIO on GitHub](https://github.com/serge1/ELFIO)
- [ELF Specification](https://refspecs.linuxbase.org/elf/elf.pdf)

---

*ELFIO and ARIO: Empowering your C++ binary and archive manipulation with simplicity, performance, and reliability!*
