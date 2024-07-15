#include "CoEditor.h"
#include <chrono>
#include <string>
#include <thread>
#include <vector>
using namespace std;
CoEditor::CoEditor(BoundedBuffer *typeBuffer, UnboundedBuffer *displayBuffer) : typeBuffer(typeBuffer), displayBuffer(displayBuffer) {}

void CoEditor::edit() {
    while (true) {
        string toEdit = typeBuffer->remove();
        if (toEdit == "DONE") {
            displayBuffer->insert("DONE");
            break;
        }
        if (toEdit == "") {
            continue;
        }

        displayBuffer->insert(toEdit);
        // sleep 0.1 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}