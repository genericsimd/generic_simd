#FAQ and Trouble Shooting

## Frequently asked questions

1. What target SIMD platforms does the library support?

Currently we support two SIMD platforms, SSE4.2 and VSX. We also
support a generic implementation of the library using scalar codes.

2. Failed to build unit tests under tests/

```
-bash-4.1$ make
g++ -Igtest-1.6.0/include -Igtest-1.6.0 -c gtest-1.6.0/src/gtest-all.cc
g++: error: gtest-1.6.0/src/gtest-all.cc: No such file or directory
g++: fatal error: no input files
```

Our unit test engines uses google test framework. Due to opensource
license issues, googletest is not included in our source tree. Please
download googletest from [here](https://code.google.com/p/googletest/)
and unzip it into "tests/gtest-1.6.0/". Or you can unzip it to
where you want, and modify the "GTEST_DIR" value in tests/Makefile.

3. Could I get slightly different results using svec_madd and
svec_msub on different platforms?

The vsx's vsx::svec_madd(), vsx::svec_msub() are mapped into madd and
msub intrinsics directly, while generic::svec_madd,
generic::svec_msub() is implemented by scalar code. In rare occasions,
fused operation by one hardware instruction provides higher precision
in float operations. So it's possible the vsx and generic provide
slightly different results.

## Known Bugs

