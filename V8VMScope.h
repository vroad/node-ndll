#ifndef _V8VMSCOPE_H_
#define _V8VMSCOPE_H_

#include <vector>
#include <map>
#include <v8.h>
#include <assert.h>
#include <memory>

namespace
{

using namespace v8;

class V8HandleContainer;
typedef std::unique_ptr<V8HandleContainer> V8HandleContainerPtr;
typedef std::vector<V8HandleContainerPtr> V8HandleContainerList;

std::map<Isolate*, std::unique_ptr<V8HandleContainerList>> valuesMap;

class TmpHandle
{
public:

	TmpHandle(Handle<Value> value)
		: value(value)
	{
	}

	Handle<Value> value;
};

class V8HandleContainer
{
public:

	~V8HandleContainer()
	{
		for (size_t i = 0; i < handles.size(); ++i)
			delete handles[i];
	}

	std::vector<TmpHandle*> handles;
	std::vector<std::string> stringValues;
	std::vector<std::wstring> wstringValues;
	std::vector<std::vector<int>> intArrayValues;
};

V8HandleContainerList *GetV8HandleContainerList(Isolate *isolate)
{
	V8HandleContainerList *list = valuesMap[isolate].get();
	if (!list)
	{
		list = new V8HandleContainerList();
		valuesMap[isolate] = std::unique_ptr<V8HandleContainerList>(list);
	}
	return list;
}

class V8VMScope
{
public:
	V8VMScope(Isolate *isolate)
		: isolate(isolate)
	{
		V8HandleContainerList *list = GetV8HandleContainerList(isolate);
		list->push_back(std::unique_ptr<V8HandleContainer>(new V8HandleContainer()));
	}

	~V8VMScope()
	{
		V8HandleContainerList *list = GetV8HandleContainerList(isolate);
		list->pop_back();
		/*size_t handleCount = 0;
		for (size_t i = 0; i < list->size(); ++i)
			handleCount += (*list)[i].get()->handles.size();

		printf("Handles: %d, ", handleCount);*/
	}

private:
	Isolate *isolate;
};

V8HandleContainer *GetV8HandleContainer(Isolate *isolate)
{
	V8HandleContainerList *list = GetV8HandleContainerList(isolate);
	if (list->empty())
	{
		printf("Error: GetV8HandleContainer should be called inside scope.\n");
		return 0;
	}
	return (*list)[list->size() - 1].get();
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
		for (int i = 0; i < array->Length(); i++)
			intArray.push_back(array->Get(i)->Int32Value());
		
		container->intArrayValues.push_back(intArray);
		return intArray.data();
	}
	else
		return 0;
}

} // end anon namespace

#endif