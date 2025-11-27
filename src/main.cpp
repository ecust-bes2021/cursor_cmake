#include<iostream>
#include "add.h"

int main()
{
    const int a = 10;
    const int b = 20;
    const int c = add(a, b);
    std::cout << c << '\n';
    return 0;
}