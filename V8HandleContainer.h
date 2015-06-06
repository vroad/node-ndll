#ifndef _V8HANDLECONTAINER_H_
#define _V8HANDLECONTAINER_H_

#include <node.h>
#include "TmpHandle.h"

using namespace v8;

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

#endif