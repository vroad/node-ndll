#ifndef _V8WEAKHANDLEDATA_H_
#define _V8WEAKHANDLEDATA_H_

#include <node.h>
#include "hx/CFFI.h"

using namespace v8;

class V8WeakHandleData
{
public:

	V8WeakHandleData(Isolate *isolate, Handle<Value> value, hxFinalizer finalizer)
		: value(isolate, value), finalizer(finalizer)
	{
	}

	Persistent<Value> value;
	hxFinalizer finalizer;
	std::list<V8WeakHandleData*>::iterator it;
};

#endif