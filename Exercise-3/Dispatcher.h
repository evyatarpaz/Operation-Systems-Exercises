#ifndef Dispatcher_h
#define Dispatcher_h


#include <string>
#include <vector>
#include "BoundedBuffer.h"
class  Dispatcher {
private:
    std::vector<BoundedBuffer*>& producerBuffers;
    std::vector<BoundedBuffer*>& typeBuffers;



public:
    Dispatcher(std::vector<BoundedBuffer*>& producerBuffers, std::vector<BoundedBuffer*>& typeBuffers);
    void dispatch();
};




#endif // Dispatcher_h