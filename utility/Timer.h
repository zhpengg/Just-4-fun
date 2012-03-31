#ifndef __TIMEER__
#define __TIMEER__

#include <sys/time.h>

class Timer {
    public:
        Timer()
        {
            gettimeofday(&_start, NULL);
        }
        ~Timer()
        {
            struct timeval stop;
            gettimeofday(&stop, NULL);
            std::cout << "time elapse: " 
                      << ((double)(stop.tv_sec - _start.tv_sec) * 1000
                         - (stop.tv_usec - _start.tv_usec) / 1000) / 1000 
                      << std::endl;
        }
    private:
        struct timeval _start;
};

#endif // __TIMEER__
