//
// Created by Swagger on 2021/7/10.
//

#include "../include/Buffer.h"
#include <cstring>

using std::string;

Buffer::Buffer(int capacity)
:   capacity_(capacity),
    length_(0)
{
    buf_ = new char[capacity_];
}

Buffer::~Buffer() {
    delete buf_;
}


int Buffer::getSize() {
    return length_;
}

void Buffer::setSize(int len) {
    length_ = len;
}

void Buffer::reSet() {
    memset(buf_, 0, capacity_);
}

string Buffer::getString() {

    return string(buf_, length_);
}


char* Buffer::getBuf() {
    return buf_;
}





