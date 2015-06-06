#ifndef _TMPHANDLE_H_
#define _TMPHANDLE_H_

#include <node.h>

using namespace v8;

class TmpHandle
{
public:

	TmpHandle(Handle<Value> value)
		: value(value)
	{
	}

	Handle<Value> value;
};

#endif