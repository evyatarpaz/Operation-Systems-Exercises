#include "BoundedBuffer.h"
using namespace std; 

BoundedBuffer::BoundedBuffer(int size) : full(0), empty(size) {}

BoundedBuffer::~BoundedBuffer() {}


void BoundedBuffer::insert(const string& s) {
	empty.acquire();
	mutex.acquire();
	buffer.push(s);
	mutex.release();
	full.release();
}

string BoundedBuffer::remove() {
	if(!full.try_acquire()) {
		return "";
	}
	if(!mutex.try_acquire()){
		return "";
	}
	string s = buffer.front();
	buffer.pop();
	mutex.release();
	empty.release();
	return s;
}