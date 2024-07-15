#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <queue>
#include <string>
#include <mutex>
#include <semaphore>
#include <limits.h>

class BoundedBuffer {
private:
    std::queue<std::string> buffer;
    std::counting_semaphore<INT_MAX> full;
    std::counting_semaphore<INT_MAX> empty;
    std::binary_semaphore mutex{1};             

public:
    BoundedBuffer(int size);
    ~BoundedBuffer();
    void insert(const std::string& s);
    std::string remove();
};

#endif // BoundedBuffer_h
