#include <iostream>

#ifndef I_AM_SO_COOL
#define I_AM_SO_COOL 0
#endif

int main(int argc, char** argv)
{
    std::cout << "Hello, World!" << std::endl;

    for (int i = 0; i < argc; i++)
        std::cout << "[ARG " << i << "]: " << argv[i] << std::endl;

    std::cout << (I_AM_SO_COOL ? "HELL YEAH!! I'M COOOL" : "I'M NOT COOL :(") << std::endl;

    return 0;
}
