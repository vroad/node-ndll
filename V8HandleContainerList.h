#ifndef _V8HANDLECONTAINERLIST_H_
#define _V8HANDLECONTAINERLIST_H_

#include <node.h>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "AbstractData.h"
#include "V8WeakHandleData.h"
#include "V8HandleContainer.h"

using namespace v8;

typedef std::unique_ptr<V8HandleContainer> V8HandleContainerPtr;
typedef std::unique_ptr<AbstractData> AbstractDataPtr;
typedef std::map<std::string, int> NameToID;
typedef std::map<std::string, int> KindMap;

typedef std::unique_ptr<TmpHandle> TmpHandlePtr;

class V8HandleContainerList
{
public:

	V8HandleContainerList(Isolate *isolate);

	~V8HandleContainerList();

	void Dispose();

	void PushHandleContainer();

	void PopHandleContainer();

	TmpHandle *PushHandle(Handle<Value> value);

	const char *PushString(Handle<Value> value);

	const wchar_t *PushWString(Handle<Value> value);

	int *PushIntArray(Handle<Value> value);

	TmpHandle *GetHandleFromPool(Handle<Value> value);

	void ReturnHandleToPool(TmpHandle *handle);

	Isolate *isolate;
	std::vector<V8HandleContainerPtr> containers;
	size_t containerListSize;

	std::vector<TmpHandle*> handles;
	std::vector<std::string> stringValues;
	std::vector<std::wstring> wstringValues;
	std::vector<std::vector<int>> intArrayValues;

	std::vector<TmpHandle*> handlePool;

	NameToID sgNameToID;
	std::vector< std::string > sgIDToName;
	std::vector< Persistent<String, CopyablePersistentTraits<String>> > sgIDToHandle;
	std::list< V8WeakHandleData* > weakHandles;
	std::vector< AbstractDataPtr > abstractDataList;

	bool disposed;
};

#endif