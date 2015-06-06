#include "HandleUtils.h"

static std::map<Isolate*, V8HandleContainerList*> valuesMap;

V8HandleContainerList *GetV8HandleContainerList(Isolate *isolate)
{
	V8HandleContainerList *list = valuesMap[isolate];
	if (!list)
	{
		list = new V8HandleContainerList(isolate);
		valuesMap[isolate] = list;
	}
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

void DisposeValuesMap()
{
	for (std::map<Isolate*, V8HandleContainerList*>::iterator it = valuesMap.begin(); it != valuesMap.end(); ++it)
	{
		V8HandleContainerList *list = (*it).second;
		list->Dispose();
		delete list;
	}
}