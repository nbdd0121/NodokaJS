#include "c/math.h"

#include "js/builtin.h"
#include "js/object.h"

static enum nodoka_completion prototype_toString(nodoka_context *C, nodoka_data *this, nodoka_data **ret, char *defName) {
    if (this->type != NODOKA_OBJECT) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: *Error.prototype.toString called on non-object");
        return NODOKA_COMPLETION_THROW;
    }
    nodoka_object *thisObj = (nodoka_object *)this;
    nodoka_data *named = nodoka_get(thisObj, nodoka_newStringFromUtf8("name"));
    nodoka_string *names;
    if (named->type == NODOKA_UNDEF) {
        names = nodoka_newStringFromUtf8(defName);
    } else {
        names = nodoka_toString(C, named);
    }
    nodoka_data *msgd = nodoka_get(thisObj, nodoka_newStringFromUtf8("message"));
    nodoka_string *msgs;
    if (msgd->type == NODOKA_UNDEF) {
        msgs = nodoka_newStringFromUtf8("");
    } else {
        msgs = nodoka_toString(C, msgd);
    }
    if (!names->value.len) {
        *ret = (nodoka_data *)msgs;
        return NODOKA_COMPLETION_RETURN;
    }
    if (!msgs->value.len) {
        *ret = (nodoka_data *)names;
        return NODOKA_COMPLETION_RETURN;
    }
    *ret = (nodoka_data *)nodoka_concatString(3, names, nodoka_newStringFromUtf8(": "), msgs);
    return NODOKA_COMPLETION_RETURN;
}

#define DEFINE_NATIVE_ERROR(name) nodoka_object *nodoka_new##name(nodoka_global *global, nodoka_string* msg) {\
        nodoka_object *obj = nodoka_newNativeObject();\
        obj->_class = nodoka_newStringFromUtf8("Error");\
        obj->prototype = global->name##_prototype;\
        if(msg){\
            nodoka_global_defineValue(obj, "message", (nodoka_data*)msg, true, false, true);\
        }\
        return obj;\
    }\
    \
    static enum nodoka_completion name##_construct(nodoka_context *C, nodoka_object *func, nodoka_data **ret, int argc, nodoka_data **argv) {\
        if (argc != 0 && argv[0]->type != NODOKA_UNDEF) {\
            *ret=(nodoka_data*)nodoka_new##name(C->global, nodoka_toString(C, argv[0]));\
        }else{\
            *ret=(nodoka_data*)nodoka_new##name(C->global, NULL);\
        }\
        return NODOKA_COMPLETION_RETURN;\
    }\
    static enum nodoka_completion name##_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {\
        return name##_construct(C, func, ret, argc, argv);\
    }\
    static enum nodoka_completion name##_prototype_toString(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {\
        return prototype_toString(C, this, ret, #name);\
    }\
    void nodoka_newGlobal_##name(nodoka_global *global) {\
        nodoka_object *prototype = nodoka_new##name(global, NULL);\
        prototype->prototype = global->Object_prototype;\
        global->name##_prototype = prototype;\
        nodoka_object *name = nodoka_newNativeFunction(global, name##_native, 1);\
        name->construct = name##_construct;\
        global->name = name;\
        nodoka_global_defineValue(name, "prototype", (nodoka_data *)prototype, false, false, false);\
        nodoka_global_defineValue(prototype, "constructor", (nodoka_data *)name, true, false, true);\
        nodoka_global_defineValue(prototype, "name", (nodoka_data *)nodoka_newStringFromUtf8(#name), true, false, true);\
        nodoka_global_defineValue(prototype, "message", (nodoka_data *)nodoka_newStringFromUtf8(""), true, false, true);\
        nodoka_global_defineFunc(global, prototype, "toString", name##_prototype_toString, 0, true, false, true);\
    }

DEFINE_NATIVE_ERROR(Error);
DEFINE_NATIVE_ERROR(ReferenceError);
DEFINE_NATIVE_ERROR(TypeError);
