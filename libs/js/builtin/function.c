#include "c/math.h"

#include "js/builtin.h"
#include "js/object.h"

static enum nodoka_completion newFunction_native(nodoka_context *C, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv) {
    *ret = (nodoka_data *)nodoka_newStringFromUtf8("UnsupportedError: Cannot create new function");
    return NODOKA_COMPLETION_THROW;
}

static enum nodoka_completion function_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    return newFunction_native(C, func, ret, argc, argv);
}

nodoka_object *nodoka_newNativeFunction(nodoka_global *global, nodoka_call_func func, uint32_t argc) {
    nodoka_object *function = nodoka_newNativeObject();
    function->_class = nodoka_newStringFromUtf8("Function");
    function->prototype = global->Function_prototype;
    //TODO function->get
    function->call = func;

    nodoka_global_defineValue(function, "length", (nodoka_data *)nodoka_newNumber(argc), false, false, false);
    return function;
}

static enum nodoka_completion prototype_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    *ret = nodoka_undefined;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion prototype_toString(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (this->type == NODOKA_OBJECT) {
        nodoka_object *obj = (nodoka_object *)this;
        if (obj->call) {
            if (obj->codeString) {
                *ret = (nodoka_data *)obj->codeString;
            } else {
                if (obj->code) {
                    *ret = (nodoka_data *)nodoka_newStringFromUtf8("function () { [bytecode] }");
                } else {
                    *ret = (nodoka_data *)nodoka_newStringFromUtf8("function () { [native code] }");
                }
            }
            return NODOKA_COMPLETION_RETURN;
        }
    }
    *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Function.prototype.toString is not generic");
    return NODOKA_COMPLETION_THROW;
}

void nodoka_newGlobal_Function(nodoka_global *global) {
    global->Function_prototype = NULL;
    nodoka_object *prototype = nodoka_newNativeFunction(global, prototype_native, 0);
    global->Function_prototype = prototype;
    prototype->codeString = nodoka_newStringFromUtf8("function Empty() {}");

    nodoka_object *function = nodoka_newNativeFunction(global, function_native, 1);
    function->construct = newFunction_native;
    global->function = function;

    nodoka_global_defineValue(function, "prototype", (nodoka_data *)prototype, false, false, false);

    nodoka_global_defineValue(prototype, "constructor", (nodoka_data *)function, true, false, true);
    nodoka_global_defineFunc(global, prototype, "toString", prototype_toString, 0, true, false, true);
}