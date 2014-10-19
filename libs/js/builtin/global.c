#include "c/math.h"
#include "c/stdlib.h"

#include "js/builtin.h"
#include "js/object.h"
#include "js/pass.h"

nodoka_prop_desc *nodoka_createDataDesc(nodoka_data *val, bool writable, bool enumerable, bool configurable) {
    nodoka_prop_desc *desc = nodoka_newPropertyDesc();
    desc->value = val;
    desc->writable = writable ? nodoka_true : nodoka_false;
    desc->enumerable = enumerable ? nodoka_true : nodoka_false;
    desc->configurable = configurable ? nodoka_true : nodoka_false;
    return desc;
}

void nodoka_global_defineValue(nodoka_object *object, char *name, nodoka_data *data, bool w, bool e, bool c) {
    nodoka_defineOwnProperty(object,
                             nodoka_newStringFromUtf8(name),
                             nodoka_createDataDesc(data, w, e, c),
                             true);
}

void nodoka_global_defineFunc(nodoka_global *G, nodoka_object *object, char *name, nodoka_call_func func, uint32_t argc, bool w, bool e, bool c) {
    nodoka_global_defineValue(object, name, (nodoka_data *)nodoka_newNativeFunction(G, func, argc), w, e, c);
}

static enum nodoka_completion isNaN_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = nodoka_true;
    } else {
        *ret = isnan(nodoka_toNumber(argv[0])->value) ? nodoka_true : nodoka_false;
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion isFinite_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = nodoka_false;
    } else {
        double val = nodoka_toNumber(argv[0])->value;
        *ret = isfinite(val) ? nodoka_true : nodoka_false;
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion eval(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = nodoka_undefined;
        return NODOKA_COMPLETION_RETURN;
    } else if (argv[0]->type != NODOKA_STRING) {
        *ret = argv[0];
        return NODOKA_COMPLETION_RETURN;
    }
    nodoka_string *evalBody = (nodoka_string *)argv[0];
    nodoka_code *code = nodoka_compile(evalBody->value);

    if (false) {
        assert(!"Direct Call");
    } else {
        nodoka_context *context = nodoka_newContext(C->global, C->env, code, C->this);
        enum nodoka_completion comp = nodoka_exec(context, ret);
        nodoka_disposeContext(context);
        nodoka_disposeCode(code);
        return comp;
    }
}

void nodoka_newGlobal(nodoka_global *scope) {

    nodoka_newGlobal_Function(scope);
    nodoka_newGlobal_Object(scope);
    scope->Function_prototype->prototype = scope->Object_prototype;

    nodoka_newGlobal_Array(scope);
    nodoka_newGlobal_String(scope);
    nodoka_newGlobal_Error(scope);
    nodoka_newGlobal_ReferenceError(scope);

    nodoka_object *global = nodoka_newObject(scope);
    scope->global = global;

    nodoka_global_defineValue(global, "Function", (nodoka_data *)scope->function, true, false, true);
    nodoka_global_defineValue(global, "Object", (nodoka_data *)scope->object, true, false, true);
    nodoka_global_defineValue(global, "Array", (nodoka_data *)scope->Array, true, false, true);
    nodoka_global_defineValue(global, "String", (nodoka_data *)scope->String, true, false, true);
    nodoka_global_defineValue(global, "Error", (nodoka_data *)scope->Error, true, false, true);
    nodoka_global_defineValue(global, "ReferenceError", (nodoka_data *)scope->ReferenceError, true, false, true);
    nodoka_global_defineValue(global, "TypeError", (nodoka_data *)scope->TypeError, true, false, true);
    nodoka_global_defineValue(global, "NaN", (nodoka_data *)nodoka_nan, false, false, false);
    nodoka_global_defineValue(global, "Infinity", (nodoka_data *)nodoka_newNumber(1.0 / 0.0), false, false, false);
    nodoka_global_defineValue(global, "undefined", nodoka_undefined, false, false, false);
    nodoka_global_defineFunc(scope, global, "eval", eval, 1, true, false, true);
    nodoka_global_defineFunc(scope, global, "isNaN", isNaN_native, 1, true, false, true);
    nodoka_global_defineFunc(scope, global, "isFinite", isFinite_native, 1, true, false, true);
    nodoka_global_defineValue(global, "__nodoka__", (nodoka_data *)nodoka_newGlobal_nodoka(scope), false, false, false);
}
