#Getting Started

##Getting the source

Clone the library from github
```bash
$ git clone https://github.com/pengwuibm/generic_simd.git gsimd
```
The package contains the following directories:

- docs/ input to doxygen and makefile to generate documents
- examples/ Examples using the library
- include/</t> The library source code
- tests/ Unit tests, test library implementation

##Using the library

The library is implemented completely inside header files, all of which are under include/. To use the library, follow these steps:

1. Include the library header <gsimd.h> into your source code
2. Programming according to library API
3. Build the binary w/ standard g++ like this:
```bash
g++ -I <gsimd_root>/include -m{vsx|sse4.2} -Wno-int-to-pointer-cast -flax-vector-conversions ...
```
  - -mvsx: standard g++ option to generate VSX instructions
  - -msse4.2: standard g++ option to generate SSE4.2 instructions
  - if no -mvsx or -msse4.2 is specified: generate scalar codes emulating generic SIMD intrinsics
  - -Wno-int-to-pointer-cast -flax-vector-conversions: ignore some warnings and enable vector casts

Consider the hello-world example:
```cpp
  HelloSIMD.cpp
#include <iostream>
#include <gsimd.h>

int main (int argc, char* argv[])
{
    svec4_f v1(1.1, 2.2, 3.3, 4.4);
    svec4_f v2 = v1 * 2;
    std::cout << "Hello World: " << v2 << std::endl;
    return 0;
}
```

Example#1: how to build for VSX
```bash
$ g++ -I../../include HelloSIMD.cpp -mvsx -flax-vector-conversions -o HelloSIMD -Wno-int-to-pointer-cast
$ ./HelloSIMD
Hello World: svec4_f[2.2, 4.4, 6.6, 8.8]
```

Eample#2: how to build for SSE4.2
```
$ g++ -I../../include HelloSIMD.cpp -msse4.2 -o HelloSIMD -Wno-int-to-pointer-cast
$ ./HelloSIMD
Hello World: svec4_f[2.2, 4.4, 6.6, 8.8]
```

##Running examples

We provided a few examples under examples/, that includes:

- HelloSIMD hello-world example
- mandelbrot mandelbrot algorithm
- RGB2Gray RGB to gray conversion

To try out these examples, simply
```bash
$ cd examples/RGB2Gray
$ make      
$ make run
```
