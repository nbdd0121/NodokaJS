#ifndef JS_BUILTIN_H
#define JS_BUILTIN_H

#include "js/bytecode.h"
#include "js/object.h"

nodoka_prop_desc *nodoka_createDataDesc(nodoka_data *val, bool writable, bool enumerable, bool configurable);
void nodoka_global_defineValue(nodoka_object *object, char *name, nodoka_data *data, bool w, bool e, bool c);
void nodoka_global_defineFunc(nodoka_global *G, nodoka_object *object, char *name, nodoka_call_func func, uint32_t argc, bool w, bool e, bool c);

nodoka_object *nodoka_newNativeFunction(nodoka_global *global, nodoka_call_func func, uint32_t argc);

void nodoka_newGlobal(nodoka_global *global);
nodoka_object *nodoka_newGlobal_nodoka(nodoka_global *global);
void nodoka_newGlobal_Object(nodoka_global *global);
void nodoka_newGlobal_Function(nodoka_global *global);
void nodoka_newGlobal_Array(nodoka_global *global);

#endif
