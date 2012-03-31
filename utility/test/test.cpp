#include <iostream>
#include <functional>
#include "Timer.h"
#include "RandomKey.h"

int main()
{
    Timer t;
    for (int i = 0; i < 1000000; i++) {
        std::string key = RandomKey::GetKey(32);
        //std::cout << key << " -> ";
        //std::cout << std::hash<std::string>()(key) << std::endl;
        std::hash<std::string>()(key);
    }
    return 0;
}
