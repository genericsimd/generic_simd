
/*
 *  g++ -I../../include HelloSIMD.cpp -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -o HelloSIMD
 * */

#include <iostream>
#include <gsimd.h>

int main (int argc, char* argv[])
{
    svec4_f v1(1.1, 2.2, 3.3, 4.4);
    svec4_f v2 = v1 * 2;
    std::cout << "Hello, " << v2 << std::endl;
    return 0;
}

