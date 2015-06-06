#include "V8HandleContainerList.h"
#include "V8VMScope.h"

V8HandleContainerList::V8HandleContainerList(Isolate *isolate)
	: isolate(isolate)
{
}

V8HandleContainerList::~V8HandleContainerList()
{
}

void V8HandleContainerList::Dispose()
{
	HandleScope scope(isolate);
	Isolate::Scope isolateScope(isolate);
	V8VMScope v8vmScope(isolate);
	for (std::list< V8WeakHandleData* >::const_iterator it = weakHandles.begin(); it != weakHandles.end(); ++it)
	{
		V8WeakHandleData *data = (*it);
		TmpHandle handle(Local<Value>::New(isolate, data->value));
		data->finalizer((value)&handle);
		delete data;
	}
}