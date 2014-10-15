#include "c/math.h"

#include "js/object.h"

nodoka_object *nodoka_newGlobal_nodoka(void);
nodoka_object *nodoka_newGlobal_Object(void);

nodoka_prop_desc *nodoka_createDataDesc(nodoka_data *val, bool writable, bool enumerable, bool configurable) {
    nodoka_prop_desc *desc = nodoka_newPropertyDesc();
    desc->value = val;
    desc->writable = writable ? nodoka_true : nodoka_false;
    desc->enumerable = enumerable ? nodoka_true : nodoka_false;
    desc->configurable = configurable ? nodoka_true : nodoka_false;
    return desc;
}

static nodoka_object *createFunc() {
    return nodoka_newObject();
}

static enum nodoka_completion isNaN_native(nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = nodoka_true;
    } else {
        *ret = isnan(nodoka_toNumber(argv[0])->value) ? nodoka_true : nodoka_false;
    }
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion isFinite_native(nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = nodoka_false;
    } else {
        double val = nodoka_toNumber(argv[0])->value;
        *ret = isfinite(val) ? nodoka_true : nodoka_false;
    }
    return NODOKA_COMPLETION_RETURN;
}


nodoka_object *nodoka_newGlobal(void) {
    nodoka_object *global = nodoka_newObject();
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("Object"),
                             nodoka_createDataDesc((nodoka_data *)nodoka_newGlobal_Object(), true, false, true),
                             true);
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("NaN"),
                             nodoka_createDataDesc((nodoka_data *)nodoka_nan, false, false, false),
                             true);
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("Infinity"),
                             nodoka_createDataDesc((nodoka_data *)nodoka_newNumber(1.0 / 0.0), false, false, false),
                             true);
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("undefined"),
                             nodoka_createDataDesc(nodoka_undefined, false, false, false),
                             true);
    nodoka_object *isNaN = createFunc();
    isNaN->call = isNaN_native;
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("isNaN"),
                             nodoka_createDataDesc((nodoka_data *)isNaN, true, false, true),
                             true);
    nodoka_object *isFinite = createFunc();
    isFinite->call = isFinite_native;
    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("isFinite"),
                             nodoka_createDataDesc((nodoka_data *)isFinite, true, false, true),
                             true);

    nodoka_defineOwnProperty(global,
                             nodoka_newStringFromUtf8("__nodoka__"),
                             nodoka_createDataDesc((nodoka_data *)nodoka_newGlobal_nodoka(), false, false, false),
                             true);
    return global;
}