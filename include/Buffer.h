//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_BUFFER_H
#define FROMZEROSTARTRTSP_BUFFER_H
#include <iostream>

class Buffer{
public:

    Buffer(int capacity);

    ~Buffer();

    int getSize();
    void setSize(int len);
    void reSet();
    std::string getString();
    char* getBuf();
    inline int getCapacity(){return capacity_;}

private:
    int capacity_;
    char* buf_;
    int length_;

};

#endif //FROMZEROSTARTRTSP_BUFFER_H
