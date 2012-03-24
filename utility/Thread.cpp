#include "Thread.h"
#include <unistd.h>
#include <sys/syscall.h>

Thread::Thread(ThreadFunc func, void *data): _func(func), _data(data)
{
}

Thread::~Thread()
{
}

bool Thread::start()
{
    if (pthread_create(&_thread, NULL, Thread::_run, this) != 0) {
        return false;
    }
    return true;
}

bool Thread::join()
{
    if (pthread_join(_thread, NULL)  != 0) {
        return false;
    }
    return true;
}

bool Thread::detach()
{
    if (pthread_detach(_thread) != 0) {
        return false;
    }
    return true;
}

void *Thread::data()
{
    return _data;
}

pid_t Thread::gettid()
{
    return syscall(SYS_gettid);
}

void *Thread::_run(void *arg)
{
    Thread *t = static_cast<Thread*>(arg);
    t->_func(arg);
    return NULL;
}

#ifndef DEBUG
#include <iostream>

void print_number(void *arg)
{
    Thread *t = static_cast<Thread *>(arg);
    std::cout << *(static_cast<int*>(t->data())) << std::endl;
    std::cout << t->gettid() << std::endl;
}

int main()
{
    int arr[] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 5; ++i) {
        Thread t(print_number, &arr[i]);
        t.start();
        t.join();
    }

    Thread t(print_number, arr);
    t.start();
    t.detach();

    Thread t1(print_number, arr+3);
    t1.start();
    t1.join();

    //Thread t2(t);
    //t = t1;

    return 0;
}

#endif
