#Generic SIMD Library

The Generic SIMD Library allowers users to write C++ SIMD codes that are portable across different SIMD ISAs.

##Running examples
```c++
//HelloSIMD.cpp
#include <iostream>
#include <gsimd.h>
    
int main (int argc, char* argv[])
{
  svec<4,float> v1(1.1, 2.2, 3.3, 4.4);
  svec<4,float> v2 = v1 * 2;
  std::cout << "Hello World: " << v2 << std::endl;
  return 0;
}
```

Let's use the example above to illustrate some of the basics features of the library:
- The entire generic SIMD library is included from the header file <gsimd.h>.
- Using proper platform-specific compiler flags, the code can be compiled by standard G++ into binaries for different target SIMD architectures.
- In this example, svec<4,float> is the SIMD vector abstraction provided by the library. It represents a vector of 4 floating-point values.
- Most operations on SIMD vectors use standard C++ operators such as "*" and "<<".

##Key features

The library provides:
- <b>Fixed-lane SIMD vectors.</b> Our SIMD vectors are defined based on the number of elements per vector (<i>fixed-lane</i>) instead of the byte-length of a vector (<i>fixed-width</i>). This is the key diffence between our vector types and the ones defined in platform-specific intrinsics.

   We choose fixed-lane vector because it is more natural to SIMDized parallel loops that involve data of different length such as int and double.

   We intend to support vectors with arbitrary power-of-two lanes, but currently only 4-element vectors are supported. Vector of 2-, and 8-elements are under development.

- <b>Portable SIMD programming.</b> The programming interface of the library is completely platform neutral. The library provides mapping from the interface to target SIMD platforms. The current release supports the following target platforms:
  + SSE4.2
  + VSX for P7
  + Scalar emulation for non-SIMD platforms

- <b>Overloaded C++ semantics on SIMD vectors.</b> We define SIMD vector operations based on semantics of C++ operators instead of platform-specific ISA semantics. This is because the semantics of C++ operators are platform independent. Secondly, C++ operators provide a slightly higher semantics than platform-specific intrinsics and are more natural to program since most users understand C++ operators well.

##More Information
- [Generic SIMD Intrinsics Library API](http://genericsimd.github.io/generic_simd/index.html)
- [Generic SIMD API Guide](http://genericsimd.github.io/generic_simd/apiguide/apiguide.html)
- [Getting Started](docs/getting_started.md)
- [Programming Guide](docs/programming_guide.md)
- [Developer Guide](docs/developer_guide.md)
- [WPMVP2014 Paper] (https://github.com/genericsimd/generic_simd/raw/master/)
- [FAQ & Trouble Shooting](docs/faq.md)
- [Performance Data](docs/performance.md)
- [History](docs/history.md)

