/**
 * @file BufferDump.h
 * @brief An imsi record buffer with async thread dump
 * @author zhpeng.is@gmail.ocm
 * @version 0.0.1
 * @date 2012-03-23
 */
#ifndef __BUFFER_DUMP__
#define __BUFFER_DUMP__

#include <vector>
#include <string>
#include <map>

#include <pthread.h>
using namespace std;

class BufferDump {
    public:
        typedef vector<char> packet_type;

        int init(vector<string> &imsi_record, size_t buffer_size);
        void push(string imsi, packet_type &packet);

    private:
        size_t _buffer_size;
        size_t _shift_size;
        vector<string> _imsi_record;
        map<string, vector<packet_type> > _buffer_list;

        void _dump_record(vector<packet_type> &buff, string imsi);
};

#endif // __BUFFER_DUMP__
