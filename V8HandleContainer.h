#ifndef _V8HANDLECONTAINER_H_
#define _V8HANDLECONTAINER_H_

#include <node.h>
#include "TmpHandle.h"

using namespace v8;

class V8HandleContainer
{
public:

	V8HandleContainer(size_t handleIndex, size_t stringIndex, size_t wstringIndex, size_t intArrayIndex)
		: handleIndex(handleIndex),
		stringIndex(stringIndex),
		wStringIndex(wstringIndex),
		intArrayIndex(intArrayIndex)
	{
	}

	~V8HandleContainer()
	{
	}

	size_t handleIndex;
	size_t stringIndex;
	size_t wStringIndex;
	size_t intArrayIndex;
};

#endif