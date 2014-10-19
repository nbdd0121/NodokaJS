#include "c/math.h"

#include "js/builtin.h"
#include "js/object.h"

nodoka_object *nodoka_newArray(nodoka_global *global, uint32_t length) {
    nodoka_object *obj = nodoka_newNativeObject();
    obj->_class = nodoka_newStringFromUtf8("Array");
    obj->prototype = global->Array_prototype;

    nodoka_global_defineValue(obj, "length", (nodoka_data *)nodoka_newNumber(length), true, false, false);
    return obj;
}

static enum nodoka_completion Array_construct(nodoka_context *C, nodoka_object *func, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 1 && argv[0]->type == NODOKA_NUMBER) {
        nodoka_number *num = (nodoka_number *)argv[0];
        uint32_t len = nodoka_toUint32(num);
        if (len != num->value) {
            *ret = (nodoka_data *)nodoka_newStringFromUtf8("RangeError: Invalid array length");
            return NODOKA_COMPLETION_THROW;
        }
        *ret = (nodoka_data *)nodoka_newArray(C->global, len);
        return NODOKA_COMPLETION_RETURN;
    }
    nodoka_object *array = nodoka_newArray(C->global, argc);
    for (int i = 0; i < argc; i++) {
        nodoka_defineOwnProperty(array,
                                 nodoka_newStringFromDouble(i),
                                 nodoka_createDataDesc(argv[i], true, true, true),
                                 true);
    }
    *ret = (nodoka_data *)array;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion Array_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    return Array_construct(C, func, ret, argc, argv);
}

void nodoka_newGlobal_Array(nodoka_global *global) {
    nodoka_object *prototype = nodoka_newArray(global, 0);
    prototype->prototype = global->Object_prototype;
    global->Array_prototype = prototype;

    nodoka_object *Array = nodoka_newNativeFunction(global, Array_native, 1);
    Array->construct = Array_construct;
    global->Array = Array;

    nodoka_global_defineValue(Array, "prototype", (nodoka_data *)prototype, false, false, false);
}