
#include "BoundedBuffer.h"
#include "CoEditor.h"
#include "Dispatcher.h"
#include "Producer.h"
#include "ScreenManager.h"
#include "UnboundedBuffer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

void extractData(const string &filePath, vector<int> &id, vector<int> &numCreate, vector<int> &queueSize, int &coEditorQueueSize) {
    ifstream file(filePath);
    string line;
    vector<Producer> producers;
    if (!file.is_open()) {
        cerr << "Failed to open file." << endl;
        return;
    }

    while (getline(file, line)) {
        istringstream iss(line);
        string word;
        iss >> word;

        if (word == "PRODUCER") {
            int idPro, numberOfProducts, bufferSize;
            iss >> idPro; // Extract producer ID directly
            id.push_back(idPro);
            getline(file, line); // Next line for number of products
            numberOfProducts = stoi(line);
            numCreate.push_back(numberOfProducts);
            getline(file, line); // Next line for queue size
            size_t pos = line.find('=') + 1;
            bufferSize = stoi(line.substr(pos));
            queueSize.push_back(bufferSize);
        } else if (line.find("Co-Editor queue size") != string::npos) {
            size_t pos = line.find('=') + 1;
            coEditorQueueSize = stoi(line.substr(pos));
        }
    }

    file.close();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <config file>" << endl;
        return 1;
    }
    ifstream configFile(argv[1]);
    if (!configFile) {
        cerr << "Error opening configuration file" << endl;
        return 1;
    }

    vector<int> id;
    vector<int> numCreate;
    vector<int> queueSize;
    int coEditorQueueSize;
    extractData(argv[1], id, numCreate, queueSize, coEditorQueueSize);

    vector<Producer> producers;
    vector<BoundedBuffer *> producerQueues;
    vector<CoEditor> coEditors;
    vector<BoundedBuffer *> coEditorQueues;
    UnboundedBuffer *displayQueue = new UnboundedBuffer();
    for (int i = 0; i < (int)id.size(); i++) {
        BoundedBuffer *buffer = new BoundedBuffer(queueSize[i]);
        Producer producer(id[i], numCreate[i], buffer);
        producers.push_back(producer);
        producerQueues.push_back(buffer);
    }
    for (int i = 0; i < 3; i++) {
        BoundedBuffer *buffer = new BoundedBuffer(coEditorQueueSize);
        CoEditor coEditor(buffer, displayQueue);
        coEditors.push_back(coEditor);
        coEditorQueues.push_back(buffer);
    }
    Dispatcher dispatcher(producerQueues, coEditorQueues);
    ScreenManager screenManager(displayQueue);

    for (int i = 0; i < (int)producers.size(); i++) {
        thread(&Producer::produce, &producers[i]).detach();
    }
    for (int i = 0; i < (int)coEditors.size(); i++) {
        thread(&CoEditor::edit, &coEditors[i]).detach();
    }
    thread(&Dispatcher::dispatch, &dispatcher).detach();
    thread(&ScreenManager::display, &screenManager).join();

    // Clean up
    for (auto buffer : producerQueues) {
        delete buffer;
    }
    for (auto buffer : coEditorQueues) {
        delete buffer;
    }
    delete displayQueue;

    return 0;
}