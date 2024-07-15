#ifndef UnboundedBuffer_h
#define UnboundedBuffer_h

#include <queue>
#include <string>
#include <mutex>
#include <semaphore>


class UnboundedBuffer {
private:
    int size;
    std::queue<std::string> buffer;
    std::counting_semaphore<INT_MAX> full{0};
    std::binary_semaphore mutex{1};             

public:
    UnboundedBuffer();
    ~UnboundedBuffer();
    void insert(const std::string& s);
    std::string remove();
};

#endif // UnboundedBuffer_h