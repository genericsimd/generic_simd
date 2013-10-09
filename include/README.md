# Header Files Organization

The key header files is gsimd.h, which is the only header file that user codes include.

The below structure is the header file organization.
```
gsimd.h
 |
 + generic.h
 |  |
 |  + generic4.h: Generic implementation of LANES=4 
 |  + generic8.h: Generic implementation of LANES=8
 |  
 + sse4.h: Intel SSE4.2 intrinsics implementaiton of LANES=4 
 |
 +-power_vsx4.h: IBM Power VSX intrinsics implementation of LANES=4
    |
    + power7_intrinsics.h Intrinsics only available on IBM Power7 Platform
    + power8_intrinsics.h Intrinsics only available on IBM Power8 Platform

gsimd_utility.h: Common macros definitions
```
