#ifndef CoEditor_h
#define CoEditor_h

#include <string>
#include <vector>
#include "BoundedBuffer.h"
#include "UnboundedBuffer.h"

class CoEditor {
private:
    BoundedBuffer* typeBuffer;
    UnboundedBuffer* displayBuffer;
public:
    CoEditor(BoundedBuffer* typeBuffer, UnboundedBuffer* displayBuffer);
    void edit();
};

#endif // CoEditor_h