#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <signal.h>
#include "../utility/RandomKey.h"

using namespace std;

//map<string, string> smap;
map<long long, string> smap;

void sigalarm(int signo)
{
    cout << "Map Size: " << smap.size() << endl;
    exit(0);
}

int main()
{
    string s;
    signal(SIGALRM, sigalarm);

    alarm(1);

    long long counter = 0;
    s = RandomKey::GetKey(32);
    while (true) {
        smap.insert(make_pair(++counter, s));
    }

    return 0;
}
