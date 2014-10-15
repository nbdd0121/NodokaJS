#include "c/math.h"

#include "js/object.h"

nodoka_prop_desc *nodoka_createDataDesc(nodoka_data *val, bool writable, bool enumerable, bool configurable);


static nodoka_object *newObject_native(nodoka_object *O, int argc, nodoka_data **argv) {
    nodoka_data *value;
    if (argc != 0) {
        value = argv[0];
    } else {
        value = nodoka_undefined;
    }
    switch (value->type) {
        case NODOKA_OBJECT: {
            return (nodoka_object *)value;
        }
        case NODOKA_STRING:
        case NODOKA_BOOL:
        case NODOKA_NUMBER:
            return nodoka_toObject(value);
    }
    assertType(value, NODOKA_NULL | NODOKA_UNDEF);
    nodoka_object *obj = nodoka_newObject();
    obj->prototype = NULL; //TODO
    //obj->_class=NULL;//TODO
    obj->extensible = true;
    // Internal Methods shoudl be done in nodoka_newObject
    return obj;
}

static enum nodoka_completion toObject_native(nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    nodoka_data *value = nodoka_undefined;
    if (argc != 0) {
        value = argv[0];
    }
    if (value == nodoka_undefined || value == nodoka_null) {
        *ret = (nodoka_data *)newObject_native(func, argc, argv);
    } else {
        *ret = (nodoka_data *)nodoka_toObject(value);
    }
    return NODOKA_COMPLETION_RETURN;
}

nodoka_object *nodoka_newGlobal_Object(void) {
    nodoka_object *object = nodoka_newObject();
    object->call = toObject_native;
    object->construct = newObject_native;
    // object->prototype should be set later
    nodoka_defineOwnProperty(object,
                             nodoka_newStringFromUtf8("length"),
                             nodoka_createDataDesc((nodoka_data *)nodoka_one, false, false, false),
                             true);
    nodoka_object *prototype = nodoka_newObject();
    nodoka_defineOwnProperty(object,
                             nodoka_newStringFromUtf8("prototype"),
                             nodoka_createDataDesc((nodoka_data *)prototype, false, false, false),
                             true);
    return object;
}