#ifndef ScreenManager_h
#define ScreenManager_h

#include <string>
#include "UnboundedBuffer.h"

class ScreenManager {
private:
    UnboundedBuffer* displayBuffer;
public:
    ScreenManager(UnboundedBuffer* displayBuffer);
    void display();
};
#endif // ScreenManager_h