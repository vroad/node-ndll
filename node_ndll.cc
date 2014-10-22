#include <node.h>
#include <v8.h>
#define IMPLEMENT_API
#include <hx/CFFI.h>

#include <windows.h>
#include <string>
#include <vector>

#include "DynamicV8Loader.h"
#include "V8VMScope.h"
#include "Util.h"

using namespace v8;

typedef void (hx_set_loader_t)(ResolveProc);

typedef void *(func_t)();

typedef Handle<Value>*(prim_multi_t)(value *, int);
typedef Handle<Value>*(prim_0_t)();
typedef Handle<Value>*(prim_1_t)(Handle<Value>*);
typedef Handle<Value>*(prim_2_t)(Handle<Value>*, Handle<Value>*);
typedef Handle<Value>*(prim_3_t)(Handle<Value>*, Handle<Value>*, Handle<Value>*);
typedef Handle<Value>*(prim_4_t)(Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*);
typedef Handle<Value>*(prim_5_t)(Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*);
typedef Handle<Value>*(prim_6_t)(Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*, Handle<Value>*);

void CallNDLLFunc(const v8::FunctionCallbackInfo<v8::Value>& args);

struct CFuncData
{
public:
	CFuncData(HMODULE mod, func_t func, int nargs, std::string name) : mod(mod), func(func), nargs(nargs), name(name) {}

	~CFuncData()
	{
		if (mod)
			FreeLibrary(mod);
	}

	HMODULE mod;
	func_t *func;
	int nargs;
	std::string name;
};
std::vector<CFuncData*> funcDataList;

// Loads a function from a NDLL library.
void V8_LoadLibrary(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	String::Utf8Value utf8Lib(args[0]), utf8Name(args[1]);
	char lib[260];
	sprintf_s(lib, "%s.dll", ToCString(utf8Lib));
	HMODULE mod = LoadLibraryA(lib);
	if (mod == NULL)
	{
		sprintf_s(lib, "%s.ndll", ToCString(utf8Lib));
		mod = LoadLibraryA(lib);
	}
	if (mod == NULL)
	{
		printf("Could not load %s.\n", lib);
		return;
	}

	char name[260];
	int numArgs = args[2]->Int32Value();
	if (numArgs != -1)
		sprintf_s(name, "%s__%d", ToCString(utf8Name), numArgs);
	else
		sprintf_s(name, "%s__MULT", ToCString(utf8Name));
	hx_set_loader_t *dll_hx_set_loader = (hx_set_loader_t*)GetProcAddress(mod, "hx_set_loader");
	if (!dll_hx_set_loader)
	{
		printf("hx_set_loader not found: %s\n", ToCString(utf8Lib));
		FreeLibrary(mod);
		return;
	}
	dll_hx_set_loader(DynamicV8Loader);
	func_t *func = (func_t*)GetProcAddress(mod, name);
	if (!func)
	{
		printf("Function %s not found in %s\n", ToCString(utf8Name), ToCString(utf8Lib));
		FreeLibrary(mod);
		return;
	}
	CFuncData *funcData = new CFuncData(mod, func, numArgs, ToCString(utf8Name));
	args.GetReturnValue().Set(FunctionTemplate::New(isolate, CallNDLLFunc, External::New(isolate, funcData))->GetFunction());
	funcDataList.push_back(funcData);
}

// Calls a NDLL function.
void CallNDLLFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	V8VMScope v8vm_scope(isolate);
	Handle<External> data = Handle<External>::Cast(args.Data());
	CFuncData *funcData = (CFuncData*)data->Value();
	Handle<Value> *ret = 0;
	if (funcData->nargs == -1)
	{
		prim_multi_t *cfunc = (prim_multi_t*)funcData->func();
		std::vector<Handle<Value>> handles;
		std::vector<Handle<Value>*> cargs;
		//printf("%s ", funcData->name.c_str());
		for (int i = 0; i < args.Length(); ++i)
			handles.push_back(args[i]);
		for (int i = 0; i < args.Length(); ++i)
			cargs.push_back(&handles[i]);
		ret = cfunc((value*)cargs.data(), cargs.size());
	}
	else if (funcData->nargs == 0)
	{
		prim_0_t *cfunc = (prim_0_t*)funcData->func();
		ret = cfunc();
	}
	else if (funcData->nargs == 1)
	{
		prim_1_t *cfunc = (prim_1_t*)funcData->func();
		ret = cfunc(&args[0]);
	}
	else if (funcData->nargs == 2)
	{
		prim_2_t *cfunc = (prim_2_t*)funcData->func();
		ret = cfunc(&args[0], &args[1]);
	}
	else if (funcData->nargs == 3)
	{
		prim_3_t *cfunc = (prim_3_t*)funcData->func();
		ret = cfunc(&args[0], &args[1], &args[2]);
	}
	else if (funcData->nargs == 4)
	{
		prim_4_t *cfunc = (prim_4_t*)funcData->func();
		ret = cfunc(&args[0], &args[1], &args[2], &args[3]);
	}
	else if (funcData->nargs == 5)
	{
		prim_5_t *cfunc = (prim_5_t*)funcData->func();
		ret = cfunc(&args[0], &args[1], &args[2], &args[3], &args[4]);
	}
	else if (funcData->nargs == 6)
	{
		prim_6_t *cfunc = (prim_6_t*)funcData->func();
		ret = cfunc(&args[0], &args[1], &args[2], &args[3], &args[4], &args[5]);
	}

	if (ret)
	{
		Handle<Value> r = *ret;
		args.GetReturnValue().Set(r);
	}
	else
	{
		printf("Could not call function which takes %d arguments: %s\n", funcData->nargs, funcData->name.c_str());
	}
}

void Cleanup(void *arg)
{
	for (size_t i = 0; i < funcDataList.size(); ++i)
		delete funcDataList[i];
	funcDataList.clear();
	ClearSgIdToHandle();
}

void Init(Handle<Object> exports) {
	Isolate* isolate = Isolate::GetCurrent();
	exports->Set(String::NewFromUtf8(isolate, "load_lib"),
		FunctionTemplate::New(isolate, V8_LoadLibrary)->GetFunction());
	node::AtExit(Cleanup);
}

NODE_MODULE(node_ndll, Init)
