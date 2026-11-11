# SHA-256 Implementation in C

A lightweight, standalone C implementation of the **SHA-256 (Secure Hash Algorithm 256-bit)** cryptographic hash algorithm, developed from scratch without external cryptographic libraries.

## Technical Overview
This project focuses on low-level binary operations, bitwise manipulations, and memory layout techniques according to the FIPS PUB 180-4 specification.

- **Language:** Pure C (C99 compliant)
- **Scope:** Single-block message padding & hash computation (inputs up to 55 bytes)
- **Core Logic:** Implementation of right-shift/rotation operations, compression functions, and standard SHA-256 constant schedules ($K_0 \dots K_{63}$)

## Building & Running

### Prerequisites
- GCC or Clang compiler
- Standard C C-runtime library

### Compilation
To compile the project, run: `gcc main.c -o main.exe`

### Usage
To run the executable, use: `main.exe STRING`

### NIST test vector Verification
To verify that your build matches the correct implementation, run the following test vector command :
| Target (Input) | Expected Result |
| :--- | :--- |
| `"abc"` | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |
| `""` *(Empty String)* | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

## Coming Soon
Here are some upcoming features planned for future updates : 

- ➔ **Multi-Block Support:** Extending the padding mechanism to handle arbitrarily long messages (beyond the single 512-bit block limitation)
- ➔ **Interactive CLI / TUI:** Building a cleaner terminal interface to easily hash files or custom inputs on the fly
- ➔ **Graphical Interface (GUI):** A lightweight desktop companion app to visualize live hashing and byte padding steps
- ➔ **File-Based Hashing:** Adding support for hashing entire local files via file stream inputs rather than relying solely on terminal arguments
