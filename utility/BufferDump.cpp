/**
 * @file BufferDump.cpp
 * @brief 
 * @author zhpeng.is@gmail.com
 * @version 0.0.1
 * @date 2012-03-23
 */
#include "BufferDump.h"
#include <algorithm>
#include <thread>
#include <iostream>

int BufferDump::init(vector<string> &imsi_record, size_t buffer_size)
{
    _buffer_size = buffer_size;
    _shift_size = 100;
    _imsi_record.assign(imsi_record.begin(), imsi_record.end());

    for (size_t i = 0; i < _imsi_record.size(); i++) {
        _buffer_list[_imsi_record[i]] = vector<packet_type>();
        _buffer_list[_imsi_record[i]].reserve(_buffer_size + _shift_size);
    }

    return 0;
}

void BufferDump::push(string imsi, packet_type &packet)
{
    vector<string>::iterator iter = std::find(_imsi_record.begin(),
                                              _imsi_record.end(), imsi);
    if (iter == _imsi_record.end())
        return;

    vector<packet_type> &pvactor = _buffer_list[imsi];
    pvactor.push_back(packet);

    if (pvactor.size() >= _buffer_size) {
        vector<packet_type> fullbuff;
        fullbuff.swap(pvactor);
        _dump_record(fullbuff, imsi);
    }
}

void write_file(vector<BufferDump::packet_type> &buff, string imsi)
{
    cout << "Write to file(" << imsi << "):" << endl;
    cout << "Size: " << buff.size() << endl;
}

void BufferDump::_dump_record(vector<packet_type> &buff, string imsi)
{
    std::thread writer(write_file, buff, imsi);
    writer.detach();
}

