#include "V8VMScope.h"
#include "V8HandleContainerList.h"
#include "HandleUtils.h"

V8VMScope::V8VMScope(Isolate *isolate)
	: isolate(isolate)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	list->PushHandleContainer();
}

V8VMScope::~V8VMScope()
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	list->PopHandleContainer();
}