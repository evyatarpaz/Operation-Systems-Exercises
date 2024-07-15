#ifndef Producer_h
#define Producer_h
#include "BoundedBuffer.h"
class Producer {
private:
    int id;
    int numItems;
    BoundedBuffer *buffer;

public:
    Producer(int id, int numItems, BoundedBuffer *buffer);
    void produce();
};

#endif