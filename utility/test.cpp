#include <iostream>
#include "RandomKey.h"

int main()
{
    for (int i = 0; i < 10; i++) {
        std::cout << RandomKey::GetKey(32) << std::endl;
    }
    return 0;
}
