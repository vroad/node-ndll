#include "V8HandleContainerList.h"
#include "V8VMScope.h"

V8HandleContainerList::V8HandleContainerList(Isolate *isolate)
	: isolate(isolate), containerListSize(0), disposed(false)
{
	Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	abstractTemplate.Reset(isolate, templ);
}

V8HandleContainerList::~V8HandleContainerList()
{
	if (!disposed)
		printf("V8HandleContainerList::Dispose() is not called.");
}

void V8HandleContainerList::Dispose()
{
	if (disposed)
	{
		printf("V8HandleContainerList::Dispose() has been called more than once.\n");
		return;
	}

	Locker locker(isolate);
	Isolate::Scope isolateScope(isolate);
	HandleScope scope(isolate);
	V8VMScope v8vmScope(isolate);
	for (std::list< V8WeakHandleData* >::const_iterator it = weakHandles.begin(); it != weakHandles.end(); ++it)
	{
		V8WeakHandleData *data = (*it);
		Local<Value> val = Local<Value>::New(isolate, data->value);
		TmpHandle handle(val);
		if (data->finalizer)
			data->finalizer((value)&handle);
		delete data;
	}
	sgIDToHandle.clear();
	for (size_t i = 0; i < handlePool.size(); ++i)
		delete handlePool[i];
	disposed = true;
}

void V8HandleContainerList::PushHandleContainer()
{
	V8HandleContainer *container;
	if (containerListSize >= containers.size())
	{
		container = new V8HandleContainer(handles.size(), stringValues.size(), wstringValues.size(), intArrayValues.size());
		containers.push_back(V8HandleContainerPtr(container));
	}
	else
	{
		container = containers[containerListSize].get();
		container->handleIndex = handles.size();
		container->stringIndex = stringValues.size();
		container->wStringIndex = wstringValues.size();
		container->intArrayIndex = intArrayValues.size();
	}
	++containerListSize;
}

void V8HandleContainerList::PopHandleContainer()
{
	V8HandleContainer *container = containers[containerListSize - 1].get();
	for (size_t i = container->handleIndex; i < handles.size(); ++i)
		ReturnHandleToPool(handles[i]);
	handles.resize(container->handleIndex);
	stringValues.resize(container->stringIndex);
	wstringValues.resize(container->wStringIndex);
	intArrayValues.resize(container->intArrayIndex);
	//containers.pop_back();
	--containerListSize;
}

TmpHandle *V8HandleContainerList::PushHandle(Handle<Value> value)
{
	TmpHandle *handle = GetHandleFromPool(value);
	handles.push_back(handle);
	return handle;
}

const char *V8HandleContainerList::PushString(Handle<Value> value)
{
	stringValues.push_back(*String::Utf8Value(value));
	return stringValues[stringValues.size() - 1].c_str();
}

const wchar_t *V8HandleContainerList::PushWString(Handle<Value> value)
{
	wstringValues.push_back((wchar_t*)*String::Value(value));
	return wstringValues[wstringValues.size() - 1].c_str();
}

int *V8HandleContainerList::PushIntArray(Handle<Value> value)
{
	if (value->IsArray())
	{
		std::vector<int> intArray;
		Handle<Array> array = value.As<Array>();
		for (uint32_t i = 0; i < array->Length(); i++)
			intArray.push_back(array->Get(i)->Int32Value());

		intArrayValues.push_back(intArray);
		return intArray.data();
	}
	else
		return 0;
}

TmpHandle *V8HandleContainerList::GetHandleFromPool(Handle<Value> value)
{
	if (handlePool.empty())
	{
		TmpHandle *handle = new TmpHandle(value);
		return handle;
	}
	else
	{
		TmpHandle *handle = handlePool[handlePool.size() - 1];
		handlePool.pop_back();
		handle->value = value;
		return handle;
	}
}

void V8HandleContainerList::ReturnHandleToPool(TmpHandle *handle)
{
	handlePool.push_back(handle);
}
