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

V8HandleContainer *GetV8HandleContainer(Isolate *isolate)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	if (list->containers.empty())
	{
		printf("Error: GetV8HandleContainer should be called inside scope.\n");
		return 0;
	}
	return list->containers[list->containers.size() - 1].get();
}

TmpHandle *NewHandlePointer(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainer *container = GetV8HandleContainer(isolate);
	container->handles.push_back(new TmpHandle(value));
	return container->handles[container->handles.size() - 1];
}

const wchar_t *ToWChar(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainer *container = GetV8HandleContainer(isolate);
	container->wstringValues.push_back((wchar_t*)*String::Value(value));
	return container->wstringValues[container->wstringValues.size() - 1].c_str();
}

const char *ToChar(Isolate *isolate, Handle<Value> value)
{
	V8HandleContainer *container = GetV8HandleContainer(isolate);
	container->stringValues.push_back(*String::Utf8Value(value));
	return container->stringValues[container->stringValues.size() - 1].c_str();
}

int *ToIntArray(Isolate *isolate, Handle<Value> value)
{
	if (value->IsArray())
	{
		V8HandleContainer *container = GetV8HandleContainer(isolate);

		std::vector<int> intArray;
		Handle<Array> array = value.As<Array>();
		for (uint32_t i = 0; i < array->Length(); i++)
			intArray.push_back(array->Get(i)->Int32Value());

		container->intArrayValues.push_back(intArray);
		return intArray.data();
	}
	else
		return 0;
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