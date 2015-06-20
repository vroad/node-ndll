#ifndef _HANDLEUTILS_H_
#define _HANDLEUTILS_H_

#include <node.h>
#include "V8HandleContainerList.h"
#include "TmpHandle.h"

using namespace v8;

V8HandleContainerList *GetV8HandleContainerList(Isolate *isolate);
V8HandleContainer *GetV8HandleContainer(Isolate *isolate);

TmpHandle *NewHandlePointer(Isolate *isolate, Handle<Value> value);

const wchar_t *ToWChar(Isolate *isolate, Handle<Value> value);
const char *ToChar(Isolate *isolate, Handle<Value> value);
int *ToIntArray(Isolate *isolate, Handle<Value> value);

void DisposeHandlesOfMainThread();
void DisposeHandlesOfWorkerThreads();

#endif