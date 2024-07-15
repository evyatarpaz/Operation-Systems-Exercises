#include "Producer.h"
#include <string>
using namespace std; 

Producer::Producer(int id, int numItems, BoundedBuffer *buffer) : id(id), numItems(numItems), buffer(buffer) {}

void Producer::produce() {
    int news = 0;
    int sports = 0;
    int weather = 0;
    for(int i = 0; i < numItems; i++) {
        int random = rand()%3;
        string item = "Producer " + to_string(id);
        if(random == 0) {
            item += " SPORTS " + to_string(sports);
            sports++;
            buffer->insert(item);
            continue;
        }
        if(random == 1) {
            item += " NEWS " + to_string(news);
            news++;
            buffer->insert(item);
            continue;
        } 
        item += " WEATHER " + to_string(weather);
        weather++;
        buffer->insert(item);
    }
    buffer->insert("DONE");
}