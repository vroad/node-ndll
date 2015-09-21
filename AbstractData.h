#ifndef _ABSTRACTDATA_H_
#define _ABSTRACTDATA_H_

#ifndef IGNORE_CFFI_API_H
#define IGNORE_CFFI_API_H
#endif

#include "hx/CFFI.h"
#include <v8.h>

struct AbstractData
{
	vkind mKind;
	void  *mPayload;
};

#endif