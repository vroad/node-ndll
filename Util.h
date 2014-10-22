#ifndef __UTIL_H
#define __UTIL_H
#include <v8.h>

namespace {

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

}

#endif