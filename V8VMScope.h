#ifndef _V8VMSCOPE_H_
#define _V8VMSCOPE_H_

#include <node.h>

using namespace v8;

class V8VMScope
{
public:
	V8VMScope(Isolate *isolate);

	~V8VMScope();

private:
	Isolate *isolate;
};

#endif