#ifndef _V8HANDLECONTAINERLIST_H_
#define _V8HANDLECONTAINERLIST_H_

#include <node.h>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "V8WeakHandleData.h"
#include "V8HandleContainer.h"

using namespace v8;

typedef std::unique_ptr<V8HandleContainer> V8HandleContainerPtr;
typedef std::map<std::string, int> NameToID;

class V8HandleContainerList
{
public:

	V8HandleContainerList(Isolate *isolate);

	~V8HandleContainerList();

	void Dispose();

	Isolate *isolate;
	std::vector<V8HandleContainerPtr> containers;

	NameToID sgNameToID;
	std::vector< std::string > sgIDToName;
	std::vector< Persistent<String, CopyablePersistentTraits<String>> > sgIDToHandle;
	std::list< V8WeakHandleData* > weakHandles;
};

#endif