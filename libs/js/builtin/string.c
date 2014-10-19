#include "c/math.h"
#include "c/stdlib.h"

#include "js/builtin.h"
#include "js/object.h"

static nodoka_prop_desc *String_getOwnProperty(nodoka_object *O, nodoka_string *P) {
    nodoka_prop_desc *desc = hashmap_get(O->prop, P);
    if (desc) {
        return desc;
    }
    int32_t index = nodoka_toInt32(nodoka_toNumber((nodoka_data *)P));
    if (nodoka_toString(NULL, (nodoka_data *)nodoka_newNumber(fabs(index))) != P) {
        return NULL;
    }
    nodoka_string *str = (nodoka_string *)O->primitiveValue;
    if (str->value.len <= index) {
        return NULL;
    }
    utf16_string_t resultStr = {
        .len = 1,
        .str = malloc(sizeof(uint16_t))
    };
    resultStr.str[0] = str->value.str[index];
    return nodoka_createDataDesc((nodoka_data *)nodoka_new_string(resultStr), true, false, false);
}

nodoka_object *nodoka_newStringObject(nodoka_global *global, nodoka_string *str) {
    nodoka_object *obj = nodoka_newNativeObject();
    obj->getOwnProperty = String_getOwnProperty;
    obj->_class = nodoka_newStringFromUtf8("String");
    obj->prototype = global->String_prototype;
    obj->primitiveValue = str ? (nodoka_data *)str : (nodoka_data *)nodoka_newStringFromUtf8("");

    nodoka_global_defineValue(obj, "length", (nodoka_data *)nodoka_newNumber(str ? str->value.len : 0), false, false, false);
    return obj;
}

static enum nodoka_completion String_construct(nodoka_context *C, nodoka_object *func, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = (nodoka_data *)nodoka_newStringObject(C->global, NULL);
    } else {
        *ret = (nodoka_data *)nodoka_newStringObject(C->global, nodoka_toString(C, argv[0]));
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion String_native(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("");
    } else {
        *ret = (nodoka_data *)nodoka_toString(C, argv[0]);
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion String_fromCharCode(nodoka_context *C, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    utf16_string_t str = {
        .len = argc,
        .str = malloc(sizeof(uint16_t) * argc)
    };
    for (int i = 0; i < argc; i++) {
        str.str[i] = nodoka_toUint16(nodoka_toNumber(argv[i]));
    }
    *ret = (nodoka_data *)nodoka_new_string(str);
    return NODOKA_COMPLETION_RETURN;
}


void nodoka_newGlobal_String(nodoka_global *global) {
    nodoka_object *prototype = nodoka_newStringObject(global, 0);
    prototype->prototype = global->Object_prototype;
    global->String_prototype = prototype;

    nodoka_object *String = nodoka_newNativeFunction(global, String_native, 1);
    String->construct = String_construct;
    global->String = String;

    nodoka_global_defineValue(String, "prototype", (nodoka_data *)prototype, false, false, false);
    nodoka_global_defineFunc(global, String, "fromCharCode", String_fromCharCode, 1, false, false, false);
}