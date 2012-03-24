/**
 * @file Thread.h
 * @brief simple thread wrapper
 * @author zhpeng.is@gmail.com
 * @version 0.0.1
 * @date 2012-03-24
 */
#ifndef __THREAD__
#define __THREAD__

#include <pthread.h>
#include <sys/types.h>

class Thread {
    public:
        typedef void (*ThreadFunc)(void *arg);

        explicit Thread(ThreadFunc func, void *data);
        ~Thread();
        bool start();
        bool join();
        bool detach();
        void *data();
        pid_t gettid();
    private:

        // disable copy and assignment
        Thread(const Thread &rhs) {}
        void operator=(const Thread &rhs) {};

        static void *_run(void *arg);
        ThreadFunc _func;
        pthread_t _thread;
        void *_data;
};

#endif // __THREAD__
