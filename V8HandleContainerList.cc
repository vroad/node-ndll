#include "V8HandleContainerList.h"
#include "V8VMScope.h"

V8HandleContainerList::V8HandleContainerList(Isolate *isolate)
	: isolate(isolate), sgKinds((int)(valtAbstractBase + 2))
{
}

V8HandleContainerList::~V8HandleContainerList()
{
}

void V8HandleContainerList::Dispose()
{
	Locker locker(isolate);
	Isolate::Scope isolateScope(isolate);
	HandleScope scope(isolate);
	V8VMScope v8vmScope(isolate);
	for (std::list< V8WeakHandleData* >::const_iterator it = weakHandles.begin(); it != weakHandles.end(); ++it)
	{
		V8WeakHandleData *data = (*it);
		TmpHandle handle(Local<Value>::New(isolate, data->value));
		data->finalizer((value)&handle);
		delete data;
	}
	sgIDToHandle.clear();
}