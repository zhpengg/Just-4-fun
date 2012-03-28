#ifndef __RANDOM_KEY__
#define __RANDOM_KEY__

#include <cstdlib>
#include <string>

class RandomKey {
    public:
        static std::string GetKey(size_t len)
        {
            std::string result;
            for (int i = 0; i < len; i++) {
                result.push_back('A' + rand() % 26);
            }
            return result;
        }
};

#endif // __RANDOM_KEY__
