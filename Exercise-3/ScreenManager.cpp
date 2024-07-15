#include "ScreenManager.h"
#include <iostream>
#include <string>
using namespace std;

ScreenManager::ScreenManager(UnboundedBuffer* displayBuffer) : displayBuffer(displayBuffer) {}

void ScreenManager::display() {
    int doneCount = 0;
    while (true)
    {
        string toDisplay = displayBuffer->remove();
        if (toDisplay == "DONE")
        {
            doneCount++;
            if (doneCount == 3)
            {
                break;
            }
            continue;
        }
        if (toDisplay == "")
        {
            continue;
        }
        cout << toDisplay << endl;
    }
    cout << "DONE" << endl;
    
}