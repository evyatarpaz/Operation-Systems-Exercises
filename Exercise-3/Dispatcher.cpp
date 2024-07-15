#include "Dispatcher.h"
#include <string>
#include <vector>
using namespace std;

Dispatcher::Dispatcher(std::vector<BoundedBuffer*> &producerBuffers, std::vector<BoundedBuffer*> &typeBuffers) : producerBuffers(producerBuffers), typeBuffers(typeBuffers) {}

void Dispatcher::dispatch() {
    int done = 0;
    int numProducers = producerBuffers.size();
    int numType = typeBuffers.size();
    while (true) {
        for (int i = 0; i < numProducers; i++) {
            string item = producerBuffers[i]->remove();
            if (item == "") {
                continue;
            }
            if (item == "DONE") {
                done++;
                if (done == numProducers) {
                    for (int j = 0; j < numType; j++) {
                        typeBuffers[j]->insert("DONE");
                    }
                    return;
                }
                continue;
            }
            // extract type from item (e.g. "NEWS", "SPORTS", "WEATHER") "“producer <i> <type> <j>”"  -> <type>
            string type = item.substr(item.find(" ") + 1, item.find(" ", item.find(" ") + 1) - item.find(" ") - 1);
            // push item to the corresponding type buffer
            if (type == "SPORTS") {
                typeBuffers[0]->insert(item);
                continue;
            }
            if (type == "NEWS") {
                typeBuffers[1]->insert(item);
                continue;
            }
            typeBuffers[2]->insert(item);
        }
    }
}
