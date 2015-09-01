#include "HandleUtils.h"
#include <array>
#include <mutex>

static std::array<V8HandleContainerList*, 16> values;
static std::mutex mtx;
static unsigned int nextThreadIndex = 0;

V8HandleContainerList *GetV8HandleContainerList(Isolate *isolate)
{
	V8HandleContainerList *list;
	for (size_t i = 0; i < values.size(); ++i)
	{
		list = values[i];
		if (list != NULL && list->isolate == isolate)
			return list;
	}
	std::lock_guard<std::mutex> lock(mtx);
	list = new V8HandleContainerList(isolate);
	values[nextThreadIndex++] = list;
	return list;
}

TmpHandle *NewHandlePointer(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	return list->PushHandle(value);
}

const char *ToChar(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	return list->PushString(value);
}

const wchar_t *ToWChar(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	return list->PushWString(value);
}

int *ToIntArray(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	return list->PushIntArray(value);
}

void DisposeHandlesOfMainThread()
{
	std::lock_guard<std::mutex> lock(mtx);
	Isolate *currentIsolate = Isolate::GetCurrent();
	for (size_t i = 0; i < values.size(); ++i)
	{
		V8HandleContainerList *list = values[i];
		if (list != NULL && list->isolate == currentIsolate)
		{
			list->Dispose();
			delete list;
			values[i] = NULL;
			break;
		}
	}
}

void DisposeHandlesOfWorkerThreads()
{
	std::lock_guard<std::mutex> lock(mtx);
	Isolate *currentIsolate = Isolate::GetCurrent();
	for (size_t i = 0; i < values.size(); ++i)
	{
		V8HandleContainerList *list = values[i];
		if (list == NULL || list->isolate == currentIsolate)
			continue;
		list->Dispose();
		delete list;
		values[i] = NULL;
	}
}