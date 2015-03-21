#include <v8.h>
#include <stdio.h>
#include <node_buffer.h>
// Get headers etc.

#define IGNORE_CFFI_API_H

#include "hx/CFFI.h"
#include <vector>
#include <map>
#include <string>

#include "V8VMScope.h"

namespace {

using namespace v8;

vkind k_int32 = (vkind)valtAbstractBase;
vkind k_hash = (vkind)(valtAbstractBase + 1);
static int sgKinds = (int)(valtAbstractBase + 2);
typedef std::map<std::string, int> KindMap;
static KindMap sgKindMap;

TmpHandle *InternalError(const char *inMessage)
{
	fprintf(stderr, "Internal error:%s\n", inMessage);
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, String::NewFromUtf8(isolate, inMessage));
}



struct AbstractData
{
	vkind mKind;
	void  *mPayload;
};

int hxcpp_alloc_kind()
{
	return ++sgKinds;
}


void hxcpp_kind_share(int &ioKind, const char *inName)
{
	int &kind = sgKindMap[inName];
	if (kind == 0)
		kind = hxcpp_alloc_kind();
	ioKind = kind;
}

typedef std::map<std::string, int> NameToID;
NameToID sgNameToID;
std::vector< std::string > sgIDToName;
std::vector< Persistent<String, CopyablePersistentTraits<String>> > sgIDToHandle;

void ClearSgIdToHandle()
{
	for (size_t i = 0; i < sgIDToHandle.size(); ++i)
		sgIDToHandle[i].Reset();
}

extern "C" {


/*
This bit of Macro magic is used to define extern function pointers
in ndlls, define stub implementations that link back to the hxcpp dll
and glue up the implementation in the hxcpp runtime.

For the static link case, these functions are linked directly.
*/

void v8_hx_error()
{
	Isolate *isolate = Isolate::GetCurrent();
	isolate->ThrowException(String::NewFromOneByte(isolate, (uint8_t*)"Error"));
}


void v8_val_throw(TmpHandle * arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	fprintf(stderr, "Exception: %s\n", *String::Utf8Value(arg1->value));
	isolate->ThrowException(arg1 ? arg1->value : (Handle<Value>)Undefined(isolate));
}


void v8_hx_fail(char * arg1, char * arg2, int arg3)
{
	fprintf(stderr, "Terminal error %s, File %s, line %d\n", arg1, arg2, arg3);
	exit(1);
}



int v8_val_type(TmpHandle * _arg1)
{
	if (_arg1 == 0)
		return valtNull;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Value> arg1 = _arg1->value;
	if (arg1->IsUndefined() || arg1->IsNull())
		return valtNull;

	if (arg1->IsBoolean())
		return valtBool;

	if (arg1->IsInt32() || arg1->IsUint32())
		return valtInt;

	if (arg1->IsNumber())
		return valtFloat;


	if (arg1->IsString())
		return valtString;

	if (arg1->IsArray())
		return valtArray;

	if (arg1->IsFunction())
		return valtFunction;

	Handle<Value> h_arg1 = Handle<External>::Cast(arg1);
	if (!h_arg1.IsEmpty())
		return valtAbstractBase;

	return valtObject;
}

vkind v8_val_kind(TmpHandle * arg1)
{
	HandleScope handle_scope(Isolate::GetCurrent());
	if (arg1 == 0)
	{
		InternalError("Null value has not 'kind'");
		return 0;
	}
	Handle<External> h_arg1 = arg1->value.As<External>();

	AbstractData *data = (AbstractData *)h_arg1->Value();
	if (!data)
	{
		InternalError("Value is not abstract");
		return 0;
	}

	return data->mKind;
}


void * v8_val_to_kind(TmpHandle * arg1, vkind arg2)
{
	if (arg1 == 0 || (arg1 && arg1->value->IsExternal()))
		return 0;

	v8::HandleScope handle_scope(Isolate::GetCurrent());
	v8::Handle<External> h_arg1 = arg1->value.As<External>();
	AbstractData *data = (AbstractData *)h_arg1->Value();
	if (!data || data->mKind != arg2)
		return 0;

	return data->mPayload;
}


// don't check the 'kind' ...
void * v8_val_data(TmpHandle * arg1)
{
	if (arg1 == 0)
		return 0;

	v8::HandleScope handle_scope(Isolate::GetCurrent());
	v8::Handle<External> h_arg1 = arg1->value.As<External>();
	AbstractData *data = (AbstractData *)h_arg1->Value();
	if (!data)
		return 0;

	return data->mPayload;
}


int v8_val_fun_nargs(TmpHandle * arg1)
{
	if (arg1 == 0)
		return faNotFunction;
	v8::HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Function> func = arg1->value.As<Function>();
	if (func.IsEmpty())
		return faNotFunction;

	// TODO:
	return 0;
}




// Extract TmpHandle * type
bool v8_val_bool(TmpHandle * arg1)
{
	if (arg1 == 0) return false;
	return (*arg1->value)->BooleanValue();
}


int v8_val_int(TmpHandle * arg1)
{
	if (arg1 == 0) return 0;
	return (*arg1->value)->Int32Value();
}


double v8_val_float(TmpHandle * arg1)
{
	if (arg1 == 0) return 0.0;
	return (*arg1->value)->NumberValue();
}


double v8_val_number(TmpHandle * arg1)
{
	if (arg1 == 0) return 0.0;
	return (*arg1->value)->NumberValue();
}



// Create v8::Value * type

TmpHandle * v8_alloc_null()
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Null(isolate));
}

TmpHandle * v8_alloc_bool(bool arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Boolean::New(isolate, arg1));
}
TmpHandle * v8_alloc_int(int arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Int32::New(isolate, arg1));
}
TmpHandle * v8_alloc_float(double arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Number::New(isolate, arg1));
}
TmpHandle * v8_alloc_empty_object()
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Object::New(isolate));
}


TmpHandle * v8_alloc_abstract(vkind arg1, void * arg2)
{
	AbstractData *data = new AbstractData;
	data->mKind = arg1;
	data->mPayload = arg2;

	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, External::New(isolate, data));
}

TmpHandle * v8_alloc_best_int(int arg1) { return v8_alloc_int(arg1); }
TmpHandle * v8_alloc_int32(int arg1) { return v8_alloc_int(arg1); }



// String access
int v8_val_strlen(TmpHandle * arg1)
{
	if (arg1 == 0 || (arg1 && !(*arg1->value)->IsString())) return 0;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<String> str = arg1->value.As<String>();
	return str->Utf8Length();
}


const wchar_t * v8_val_wstring(TmpHandle * arg1)
{
	if (arg1 == 0) return L"";

	return ToWChar(Isolate::GetCurrent(), arg1->value);
}


const char * v8_val_string(TmpHandle * arg1)
{
	if (arg1 == 0) return "";

	return ToChar(Isolate::GetCurrent(), arg1->value);
}


TmpHandle * v8_alloc_string(const char * arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, String::NewFromUtf8(isolate, arg1));
}

wchar_t * v8_val_dup_wstring(TmpHandle *inVal)
{
	return (wchar_t *)v8_val_wstring(inVal);
}

char * v8_val_dup_string(TmpHandle *inVal)
{
	return (char *)v8_val_string(inVal);
}

TmpHandle *v8_alloc_string_len(const char *inStr, int inLen)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, String::NewFromUtf8(isolate, inStr, String::kNormalString, inLen));
}

TmpHandle *v8_alloc_wstring_len(const wchar_t *inStr, int inLen)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, String::NewFromTwoByte(isolate, (uint16_t*)inStr, String::kNormalString, inLen));
}

// Array access - generic
int v8_val_array_size(TmpHandle * arg1)
{
	if (!arg1 || (arg1 && !(*arg1->value)->IsArray())) return 0;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Array> array = arg1->value.As<Array>();
	return array->Length();
}


TmpHandle * v8_val_array_i(TmpHandle * arg1, int arg2)
{
	if (!arg1) return 0;
	Isolate *isolate = Isolate::GetCurrent();
	EscapableHandleScope handle_scope(isolate);
	Handle<Object> obj = arg1->value.As<Object>();
	if (obj.IsEmpty()) return 0;
	return NewHandlePointer(isolate, handle_scope.Escape(obj->Get(arg2)));
}

void v8_val_array_set_i(TmpHandle * arg1, int arg2, TmpHandle *inVal)
{
	if (!arg1) return;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Object> obj = arg1->value.As<Object>();
	if (obj.IsEmpty()) return;
	obj->Set(arg2, inVal->value);
}

void v8_val_array_set_size(TmpHandle * arg1, int inLen)
{
	if (!arg1 || inLen == 0) return;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Object> obj = arg1->value.As<Object>();
	if (obj.IsEmpty()) return;
	obj->Get(inLen - 1);
}

void v8_val_array_push(TmpHandle * arg1, TmpHandle *inValue)
{
	if (!arg1 || (arg1 && !(*arg1->value)->IsArray())) return;
	HandleScope handle_scope(Isolate::GetCurrent());
	Handle<Array> array = arg1->value.As<Array>();
	array->Set(array->Length(), inValue->value);
}


TmpHandle * v8_alloc_array(int arg1)
{
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Array::New(isolate, arg1));
}



// Array access - fast if possible - may return null
// Resizing the array may invalidate the pointer
bool * v8_val_array_bool(v8::Value * arg1)
{
	return 0;
}


int * v8_val_array_int(v8::Value * arg1)
{
	return 0;
}


double * v8_val_array_double(v8::Value * arg1)
{
	return 0;
}

value * v8_val_array_value(v8::Value * arg1)
{
	return 0;
}




// Byte arrays
// The byte array may be a string or a Array<bytes> depending on implementation
TmpHandle *v8_val_to_buffer(TmpHandle * arg1)
{
	if (!arg1)
		return 0;
	if (arg1->value->IsObject())
	{
		HandleScope handle_scope(Isolate::GetCurrent());
		Local<Object> obj = arg1->value->ToObject();
		if (obj->HasIndexedPropertiesInExternalArrayData())
			return arg1;
	}
	else if (arg1->value->IsNull() || arg1->value->IsUndefined())
		return arg1;

	return 0;
}



buffer v8_alloc_buffer(const char *inStr)
{
	// TODO:
	return 0;
}


buffer v8_alloc_buffer_len(int inLen)
{
	Isolate *isolate = Isolate::GetCurrent();
	Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, inLen);
	return (buffer)NewHandlePointer(isolate, Uint8Array::New(ab, 0, inLen));
}


value v8_buffer_val(buffer b)
{
	return (value)b;
}


value v8_buffer_to_string(buffer inBuffer)
{
	// TODO:
	return 0;
}


void v8_buffer_append(buffer inBuffer, const char *inStr)
{
	// TODO:
	return;
}


int v8_buffer_size(TmpHandle *inBuffer)
{
	if (!inBuffer)
		return 0;
	if (inBuffer->value->IsObject())
	{
		HandleScope handle_scope(Isolate::GetCurrent());
		Local<Object> obj = inBuffer->value->ToObject();
		if (obj->HasIndexedPropertiesInExternalArrayData())
			return obj->GetIndexedPropertiesExternalArrayDataLength();
	}
	return 0;
}


void v8_buffer_set_size(buffer inBuffer, int inLen)
{
	// TODO:
}


void v8_buffer_append_sub(buffer inBuffer, const char *inStr, int inLen)
{
	// TODO:
}


void v8_buffer_append_char(buffer inBuffer, int inChar)
{
	// TODO:
}


char * v8_buffer_data(TmpHandle *inBuffer)
{
	if (!inBuffer)
		return 0;
	if (inBuffer->value->IsObject())
	{
		HandleScope handle_scope(Isolate::GetCurrent());
		Local<Object> obj = inBuffer->value->ToObject();
		if (obj->HasIndexedPropertiesInExternalArrayData())
			return static_cast<char*>(obj->GetIndexedPropertiesExternalArrayData());
	}
	else if (inBuffer->value->IsNull() || inBuffer->value->IsUndefined())
		return 0;

	InternalError("Invalid Buffer");
	return 0;
}


// Append value to buffer
void v8_val_buffer(buffer inBuffer, value inValue)
{
	// TODO:
}




#define HANDLE_FUNC \
if (!arg1) \
	return InternalError("Null Function Call"); \
Isolate *isolate = Isolate::GetCurrent(); \
EscapableHandleScope handle_scope(isolate); \
Handle<Function> func = arg1->value.As<Function>(); \
if (func.IsEmpty()) \
	return InternalError("Calling non-function");



// Call Function 
TmpHandle * v8_val_call0(TmpHandle * arg1)
{
	HANDLE_FUNC
	
	Local<Value> result = func->Call(isolate->GetCurrentContext()->Global(), 0, 0);
	return NewHandlePointer(isolate, handle_scope.Escape(result));
}

TmpHandle * v8_val_call0_traceexcept(TmpHandle * arg1)
{
	HANDLE_FUNC

	v8::TryCatch try_catch;
	Local<Value> result = func->Call(isolate->GetCurrentContext()->Global(), 0, 0);
	if (*try_catch.Exception())
	{
		fprintf(stderr, "Try/Catch erroe\n");
		exit(1);
	}

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_call1(TmpHandle * arg1, TmpHandle * arg2)
{
	HANDLE_FUNC
	
	Local<Value> args[1];
	args[0] = arg2 ? arg2->value : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(isolate->GetCurrentContext()->Global(), 1, args);
	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_call2(TmpHandle * arg1, TmpHandle * arg2, TmpHandle * arg3)
{
	HANDLE_FUNC

	Local<Value> args[2];
	args[0] = arg2 ? arg2->value : (Handle<Value>)Undefined(isolate);
	args[1] = arg3 ? arg3->value : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(isolate->GetCurrentContext()->Global(), 2, args);

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_call3(TmpHandle * arg1, TmpHandle * arg2, TmpHandle * arg3, TmpHandle * arg4)
{
	HANDLE_FUNC

	Local<Value> args[3];
	args[0] = arg2 ? arg2->value : (Handle<Value>)Undefined(isolate);
	args[1] = arg3 ? arg3->value : (Handle<Value>)Undefined(isolate);
	args[2] = arg4 ? arg4->value : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(isolate->GetCurrentContext()->Global(), 3, args);

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_callN(TmpHandle * arg1, TmpHandle * arg2)
{
	//TODO:
	return 0;
	/*
	HANDLE_FUNC

	std::vector< Handle<Value> > args[1];
	args[0] = Handle<Value>(arg2);
	args[1] = Handle<Value>(arg3);
	args[2] = Handle<Value>(arg4);
	args[3] = Handle<Value>(arg5);

	return *func->Call( Handle<Object>(), 1, args);
	*/
}


#define HANDLE_MEM_FUNC \
if (!arg1) \
	return InternalError("Null object call"); \
Isolate *isolate = Isolate::GetCurrent(); \
EscapableHandleScope handle_scope(isolate); \
Local<Object> obj = arg1->value.As<Object>(); \
if (obj.IsEmpty()) \
	return InternalError("Calling non-object member"); \
Local<Value> member = obj->Get( Local<String>::New(isolate, sgIDToHandle[arg2]) ); \
if (member.IsEmpty()) \
	return InternalError("Missing member in call"); \
Local<Function> func = member.As<Function>(); \
if (func.IsEmpty()) \
	return InternalError("Member is not a function"); \


// Call object field
TmpHandle * v8_val_ocall0(TmpHandle * arg1, int arg2)
{
	HANDLE_MEM_FUNC
	Local<Value> result = func->Call(obj, 0, 0);

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_ocall1(TmpHandle * arg1, int arg2, TmpHandle * arg3)
{
	HANDLE_MEM_FUNC

	Local<Value> args[1];
	args[0] = arg3 ? (arg3->value) : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(obj, 1, args);

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_ocall2(TmpHandle * arg1, int arg2, TmpHandle * arg3, TmpHandle * arg4)
{
	HANDLE_MEM_FUNC

	Local<Value> args[2];
	args[0] = arg3 ? (arg3->value) : (Handle<Value>)Undefined(isolate);
	args[1] = arg4 ? (arg4->value) : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(obj, 2, args);

	return NewHandlePointer(isolate, handle_scope.Escape(result));
}


TmpHandle * v8_val_ocall3(TmpHandle * arg1, int arg2, TmpHandle * arg3, TmpHandle * arg4, TmpHandle * arg5)
{
	HANDLE_MEM_FUNC

	Handle<Value> args[3];
	args[0] = arg3 ? (arg3->value) : (Handle<Value>)Undefined(isolate);
	args[1] = arg4 ? (arg4->value) : (Handle<Value>)Undefined(isolate);
	args[2] = arg5 ? (arg5->value) : (Handle<Value>)Undefined(isolate);
	Local<Value> result = func->Call(obj, 3, args);

	return NewHandlePointer(isolate, handle_scope.Escape(result));

}


TmpHandle * v8_val_ocallN(TmpHandle * arg1, int arg2, TmpHandle * arg3)
{
	// TODO:
	return 0;
}



// Objects access
int v8_val_id(const char * arg1)
{
	std::string key(arg1);
	NameToID::iterator i = sgNameToID.find(key);
	if (i != sgNameToID.end())
		return i->second;
	int idx = sgIDToName.size();
	sgIDToName.push_back(key);
	Isolate *isolate = Isolate::GetCurrent();
	Persistent<String> p(isolate, String::NewFromUtf8(isolate, arg1));
	p.MarkIndependent();
	sgIDToHandle.push_back(p);
	sgNameToID[key] = idx;
	//printf("sgIDToHandle[%d] = %s\n", idx, arg1);
	return idx;
}


void v8_alloc_field(TmpHandle * arg1, int arg2, TmpHandle * arg3)
{
	if (!arg1 || !arg3)
	{
		InternalError("Null object set");
		return;
	}
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope handle_scope(isolate);
	Handle<Object> obj = Handle<Object>::Cast(arg1->value);
	if (obj.IsEmpty())
	{
		InternalError("Setting non-object member");
		return;
	}
	obj->Set(Local<String>::New(isolate, sgIDToHandle[arg2]), arg3->value);
}

TmpHandle * v8_val_field(TmpHandle * arg1, int arg2)
{
	if (!arg1)
		return InternalError("Null object get");
	Isolate *isolate = Isolate::GetCurrent();
	EscapableHandleScope handle_scope(isolate);
	Handle<Object> obj = arg1->value.As<Object>();
	if (obj.IsEmpty())
		return InternalError("Getting non-object member");

	return NewHandlePointer(isolate, handle_scope.Escape(obj->Get(Local<String>::New(isolate, sgIDToHandle[arg2]))));
}

double v8_val_field_numeric(TmpHandle * arg1, int arg2)
{
	HandleScope handle_scope(Isolate::GetCurrent());
	TmpHandle *result = v8_val_field(arg1, arg2);
	return result ? result->value->NumberValue() : 0;
}


// Abstract types
vkind v8_alloc_kind()
{
	return (vkind)hxcpp_alloc_kind();
}

void v8_kind_share(vkind *inKind, const char *inName)
{
	int k = (int)(intptr_t)*inKind;
	hxcpp_kind_share(k, inName);
	*inKind = (vkind)k;
}



// Garbage Collection
void * v8_hx_alloc(int arg1)
{
	// TODO:
	return 0;
}


void * v8_alloc_private(int arg1)
{
	// TODO:
	return 0;
}


void v8_val_gc(TmpHandle * arg1, hxFinalizer arg2)
{
	if (!arg1)
		return;
	hxFinalizer((value)arg1);
}

void v8_val_gc_ptr(void * arg1, hxPtrFinalizer arg2)
{
	// TODO:
	InternalError("val_gc_ptr Not implemented");
}

void v8_val_gc_add_root(TmpHandle **inRoot)
{
	// TODO:
	//hx::GCAddRoot(inRoot);
	InternalError("val_gc_add_root - Not implemented");
}


void v8_val_gc_remove_root(TmpHandle **inRoot)
{
	// TODO:
	//hx::GCRemoveRoot(inRoot);
	InternalError("val_gc_remove_root - Not implemented");
}


value *v8_alloc_root()
{
	return 0;
}

void v8_free_root(value *inValue)
{
}

struct _gcroot
{
	v8::Persistent<v8::Value> saved;
};

gcroot v8_create_root(TmpHandle *inValue)
{
	_gcroot *r = new _gcroot;
	if (inValue)
		r->saved.Reset(Isolate::GetCurrent(), inValue->value);
	return (gcroot)r;
}

TmpHandle *v8_query_root(gcroot inRoot)
{
	_gcroot *r = (_gcroot*)inRoot;
	Isolate *isolate = Isolate::GetCurrent();
	return NewHandlePointer(isolate, Local<Value>::New(isolate, r->saved));
}

void v8_destroy_root(gcroot inRoot)
{
	_gcroot *r = (_gcroot*)inRoot;
	if (r)
	{
		r->saved.Reset();
		delete r;
	}
}



// Used for finding functions in static libraries
int v8_hx_register_prim(wchar_t * arg1, void* arg2)
{
	// TODO:
	//__hxcpp_register_prim(arg1,arg2);
	return 0;
}

void v8_gc_enter_blocking()
{
}

void v8_gc_exit_blocking()
{
}

void v8_gc_safe_point()
{
}

void * v8_empty() { return 0; }

#define IMPLEMENT_HERE(x) if (!strcmp(inName,#x)) return (void *)v8_##x;
#define IGNORE_API(x) if (!strcmp(inName,#x)) return (void *)v8_empty;

void *DynamicV8Loader(const char *inName)
{
	IMPLEMENT_HERE(val_type)

	IMPLEMENT_HERE(alloc_null)
	IMPLEMENT_HERE(val_bool)
	IMPLEMENT_HERE(val_int)
	IMPLEMENT_HERE(val_float)
	IMPLEMENT_HERE(val_number)
	IMPLEMENT_HERE(val_string)
	IMPLEMENT_HERE(val_wstring)
	IMPLEMENT_HERE(alloc_empty_object)
	IMPLEMENT_HERE(alloc_bool)
	IMPLEMENT_HERE(alloc_int)
	IMPLEMENT_HERE(alloc_int32)
	IMPLEMENT_HERE(alloc_float)
	IMPLEMENT_HERE(alloc_string_len)
	IMPLEMENT_HERE(alloc_wstring_len)
	IMPLEMENT_HERE(val_array_i)
	IMPLEMENT_HERE(val_array_set_i)
	IMPLEMENT_HERE(val_throw)

	IGNORE_API(gc_enter_blocking)
	IGNORE_API(gc_exit_blocking)
	IGNORE_API(gc_safe_point)
	IGNORE_API(gc_add_root)
	IGNORE_API(gc_remove_root)
	IGNORE_API(gc_set_top_of_stack)
	IMPLEMENT_HERE(create_root)
	IMPLEMENT_HERE(query_root)
	IMPLEMENT_HERE(destroy_root)
	IGNORE_API(hx_register_prim)

	IMPLEMENT_HERE(val_array_int)
	IMPLEMENT_HERE(val_array_double)
	IGNORE_API(val_array_float)
	IMPLEMENT_HERE(val_array_bool)
	IMPLEMENT_HERE(val_array_push)
	IMPLEMENT_HERE(val_array_size)

	IMPLEMENT_HERE(val_call0)
	IMPLEMENT_HERE(val_call1)
	IMPLEMENT_HERE(val_call2)
	IMPLEMENT_HERE(val_call3)

	IMPLEMENT_HERE(val_field)

	IMPLEMENT_HERE(alloc_buffer_len)
	IMPLEMENT_HERE(alloc_array)
	IMPLEMENT_HERE(alloc_root)
	IMPLEMENT_HERE(alloc_abstract)
	IMPLEMENT_HERE(alloc_field)
	IMPLEMENT_HERE(val_gc)
	IMPLEMENT_HERE(val_to_kind)
	IMPLEMENT_HERE(val_id)
	IMPLEMENT_HERE(val_data)
	IMPLEMENT_HERE(val_to_buffer)
	IMPLEMENT_HERE(buffer_data)
	IMPLEMENT_HERE(buffer_val);
	IMPLEMENT_HERE(buffer_size)
	IMPLEMENT_HERE(kind_share)

	IMPLEMENT_HERE(hx_fail)

	return 0;
}


} // end extern "C"

} // end anon namspace
