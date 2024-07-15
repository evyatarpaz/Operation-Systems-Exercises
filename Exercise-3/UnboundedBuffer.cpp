#include "UnboundedBuffer.h"
using namespace std; 

UnboundedBuffer::UnboundedBuffer() {}

UnboundedBuffer::~UnboundedBuffer() {}

void UnboundedBuffer::insert(const string& s) {
	mutex.acquire();
	buffer.push(s);
	mutex.release();
	full.release();
}

string UnboundedBuffer::remove() {
	if(!full.try_acquire()) {
		return "";
	}
	if(!mutex.try_acquire()){
		return "";
	}
	string s = buffer.front();
	buffer.pop();
	mutex.release();
	return s;
}