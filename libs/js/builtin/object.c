#include "c/math.h"

#include "js/builtin.h"
#include "js/object.h"

nodoka_object *nodoka_newObject(nodoka_global *global) {
    nodoka_object *obj = nodoka_newNativeObject();
    obj->_class = nodoka_newStringFromUtf8("Object");
    obj->prototype = global->Object_prototype;
    return obj;
}

static enum nodoka_completion newObject_native(nodoka_global *global, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv) {
    nodoka_data *value;
    if (argc != 0) {
        value = argv[0];
    } else {
        value = nodoka_undefined;
    }
    switch (value->type) {
        case NODOKA_OBJECT: {
            *ret = value;
            return NODOKA_COMPLETION_RETURN;
        }
        case NODOKA_STRING:
        case NODOKA_BOOL:
        case NODOKA_NUMBER:
            *ret = (nodoka_data *)nodoka_toObject(value);
            return NODOKA_COMPLETION_RETURN;
    }
    assertType(value, NODOKA_NULL | NODOKA_UNDEF);
    nodoka_object *obj = nodoka_newObject(global);
    // Internal Methods should be done in nodoka_newObject
    *ret = (nodoka_data *)obj;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion Object_native(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    nodoka_data *value = nodoka_undefined;
    if (argc != 0) {
        value = argv[0];
    }
    if (value == nodoka_undefined || value == nodoka_null) {
        return newObject_native(global, func, ret, argc, argv);
    } else {
        *ret = (nodoka_data *)nodoka_toObject(value);
        return NODOKA_COMPLETION_RETURN;
    }
}

static enum nodoka_completion getPrototypeOf_native(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0 || argv[0]->type != NODOKA_OBJECT) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Object.getPrototypeOf called on non-object");
        return NODOKA_COMPLETION_THROW;
    }
    nodoka_object *obj = (nodoka_object *)argv[0];
    if (obj->prototype) {
        *ret = (nodoka_data *)obj->prototype;
    } else {
        *ret = nodoka_null;
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion preventExtensions_native(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0 || argv[0]->type != NODOKA_OBJECT) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Object.preventExtensions called on non-object");
        return NODOKA_COMPLETION_THROW;
    }
    nodoka_object *obj = (nodoka_object *)argv[0];
    obj->extensible = false;
    *ret = (nodoka_data *)obj;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion isExtensible_native(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0 || argv[0]->type != NODOKA_OBJECT) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Object.isExtensible called on non-object");
        return NODOKA_COMPLETION_THROW;
    }
    nodoka_object *obj = (nodoka_object *)argv[0];
    *ret = obj->extensible ? nodoka_true : nodoka_false;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion prototype_toString(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (this->type == NODOKA_UNDEF) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("[object Undefined]");
    } else if (this->type == NODOKA_NULL) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("[object Null]");
    } else {
        nodoka_object *obj = nodoka_toObject(this);
        nodoka_string *str = nodoka_newStringFromUtf8("[object ");
        str = nodoka_concatString(str, obj->_class);
        str = nodoka_concatString(str, nodoka_newStringFromUtf8("]"));
        *ret = (nodoka_data *)str;
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion prototype_valueOf(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    *ret = (nodoka_data *)nodoka_toObject(this);
    return NODOKA_COMPLETION_RETURN;
}


void nodoka_newGlobal_Object(nodoka_global *global) {
    nodoka_object *object = nodoka_newNativeFunction(global, Object_native, 1);
    global->object = object;
    nodoka_object *prototype = nodoka_newObject(global);
    prototype->prototype = NULL;
    global->Object_prototype = prototype;
    object->construct = newObject_native;

    nodoka_global_defineValue(object, "prototype", (nodoka_data *)global->Object_prototype, true, false, true);

    nodoka_global_defineFunc(global, object, "getPrototypeOf", getPrototypeOf_native, 1, true, false, true);
    nodoka_global_defineFunc(global, object, "preventExtensions", preventExtensions_native, 1, true, false, true);
    nodoka_global_defineFunc(global, object, "isExtensible", isExtensible_native, 1, true, false, true);

    nodoka_global_defineValue(prototype, "constructor", (nodoka_data *)object, true, false, true);
    nodoka_global_defineFunc(global, prototype, "toString", prototype_toString, 0, true, false, true);
    nodoka_global_defineFunc(global, prototype, "valueOf", prototype_valueOf, 0, true, false, true);
}