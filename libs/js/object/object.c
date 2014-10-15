#include "c/assert.h"

#include "js/object.h"

#include "unicode/hash.h"

int nodoka_compareString(void *a, void *b) {
    nodoka_string *x = a;
    nodoka_string *y = b;
    return unicode_utf16Cmp(&x->value, &y->value);
}

int nodoka_hashString(void *a) {
    nodoka_string *x = a;
    return unicode_utf16Hash(&x->value);
}

static nodoka_prop_desc *getOwnProperty(nodoka_object *O, nodoka_string *P) {
    nodoka_prop_desc *desc = hashmap_get(O->prop, P);
    if (!desc) {
        return NULL;
    }
    return desc;
}

nodoka_object *nodoka_newObject(void) {
    nodoka_object *obj = (nodoka_object *)nodoka_new_data(NODOKA_OBJECT);
    obj->prop = hashmap_new(nodoka_hashString, nodoka_compareString, 11);
    obj->prototype = NULL;
    obj->extensible = true;
    obj->getOwnProperty = getOwnProperty;
    obj->call = NULL;
    obj->construct = NULL;
    return obj;
}

nodoka_prop_desc *nodoka_getOwnProperty(nodoka_object *O, nodoka_string *P) {
    return O->getOwnProperty(O, P);
}

nodoka_prop_desc *nodoka_getProperty(nodoka_object *O, nodoka_string *P) {
    nodoka_prop_desc *prop = nodoka_getOwnProperty(O, P);
    if (prop) {
        return prop;
    }
    if (O->prototype == NULL) {
        return NULL;
    }
    return nodoka_getProperty(O->prototype, P);
}

nodoka_data *nodoka_get(nodoka_object *O, nodoka_string *P) {
    nodoka_prop_desc *desc = nodoka_getProperty(O, P);
    if (!desc) {
        return nodoka_undefined;
    }
    if (nodoka_isDataDescriptor(desc)) {
        return desc->value;
    } else {
        // If getter is undefined, return undefined.
        // Return the result calling the [[Call]] internal method of getter providing O as the this value and providing no
        // arguments.
        assert(0);
    }
}

bool nodoka_canPut(nodoka_object *O, nodoka_string *P) {
    nodoka_prop_desc *desc = nodoka_getOwnProperty(O, P);
    if (desc) {
        if (nodoka_isAccessorDescriptor(desc)) {
            return !!desc->set;
        } else {
            return desc->writable == nodoka_true;
        }
    }
    if (!O->prototype) {
        return O->extensible;
    }
    nodoka_prop_desc *inherited = nodoka_getProperty(O->prototype, P);
    if (!inherited) {
        return O->extensible;
    }
    if (nodoka_isAccessorDescriptor(desc)) {
        return !!desc->set;
    } else {
        if (!O->extensible) {
            return false;
        } else {
            return inherited->writable == nodoka_true;
        }
    }
}

void nodoka_put(nodoka_object *O, nodoka_string *P, nodoka_data *V, bool throw) {
    if (!nodoka_canPut(O, P)) {
        if (throw) {
            assert(!"TypeError");
        } else {
            return;
        }
    }
    nodoka_prop_desc *ownDesc = nodoka_getOwnProperty(O, P);
    if (nodoka_isDataDescriptor(ownDesc)) {
        nodoka_prop_desc *valueDesc = nodoka_newPropertyDesc();
        valueDesc->value = V;
        nodoka_defineOwnProperty(O, P, valueDesc, throw);
        return;
    }
    nodoka_prop_desc *desc = nodoka_getProperty(O, P);
    if (nodoka_isAccessorDescriptor(desc)) {
        assert(0);
    } else {
        nodoka_prop_desc *newDesc = nodoka_newPropertyDesc();
        newDesc->value = V;
        newDesc->writable = nodoka_true;
        newDesc->enumerable = nodoka_true;
        newDesc->configurable = nodoka_true;
        nodoka_defineOwnProperty(O, P, newDesc, throw);
    }
}

bool nodoka_hasOwnProperty(nodoka_object *O, nodoka_string *P);

bool nodoka_delete(nodoka_object *O, nodoka_string *P, bool throw) {
    nodoka_prop_desc *desc = nodoka_getOwnProperty(O, P);
    if (!desc) {
        return true;
    }
    if (desc->configurable == nodoka_true) {
        hashmap_remove(O->prop, P);
        return true;
    }
    if (throw) {
        assert("!TypeError");
    }
    return false;
}

nodoka_data *nodoka_defaultValue(nodoka_object *O, enum nodoka_data_type hint);

bool nodoka_defineOwnProperty(nodoka_object *O, nodoka_string *P, nodoka_prop_desc *desc, bool throw) {
    nodoka_prop_desc *current = nodoka_getOwnProperty(O, P);
    if (!current) {
        if (O->extensible) {
            if (nodoka_isGenericDescriptor(desc) || nodoka_isDataDescriptor(desc)) {
                if (!desc->value)desc->value = nodoka_undefined;
                if (!desc->writable)desc->writable = nodoka_false;
                if (!desc->enumerable)desc->enumerable = nodoka_false;
                if (!desc->configurable)desc->configurable = nodoka_false;
                assert(!desc->set);
                assert(!desc->get);
                hashmap_put(O->prop, P, desc);
            } else {
                assert(0);
            }
            return true;
        } else {
            if (throw) {
                assert(!"TypeError");
            } else {
                return false;
            }
        }
    }
    /*if(nodoka_isEmptyDescriptor(desc)){
    return true;
    }*/
    // SameValue
    if (current->configurable == nodoka_false) {
        if (desc->configurable == nodoka_true || (desc->enumerable && desc->enumerable != current->enumerable)) {
            if (throw) {
                assert(!"TypeError");
            } else {
                return false;
            }
        }
    }
    if (nodoka_isGenericDescriptor(desc)) {

    } else if (nodoka_isDataDescriptor(current) != nodoka_isDataDescriptor(desc)) {
        if (current->configurable == nodoka_false) {
            if (throw) {
                assert(!"TypeError");
            } else {
                return false;
            }
        }
        if (nodoka_isDataDescriptor(current)) {
            desc->set = nodoka_undefined;
            desc->get = nodoka_undefined;
            desc->value = NULL;
            desc->writable = NULL;
            hashmap_put(O->prop, P, desc);
        } else {
            desc->set = NULL;
            desc->get = NULL;
            desc->value = nodoka_undefined;
            desc->writable = nodoka_false;
        }
    } else if (nodoka_isDataDescriptor(desc)) {
        if (current->configurable == nodoka_false) {
            if (current->writable == nodoka_false && desc->writable == nodoka_true) {
                if (throw) {
                    assert(!"TypeError");
                } else {
                    return false;
                }
            } else if (current->writable == nodoka_false) {
                if (desc->value && true/*SameValue for Value*/) {
                    if (throw) {
                        assert(!"TypeError");
                    } else {
                        return false;
                    }
                }
            }
        }
    } else {
        if (current->configurable == nodoka_false) {
            if (true/*SameValue for Set */) {
                if (throw) {
                    assert(!"TypeError");
                } else {
                    return false;
                }
            }
            if (true/*SameValue for Get */) {
                if (throw) {
                    assert(!"TypeError");
                } else {
                    return false;
                }
            }
        }
    }
    if (desc->set)current->set = desc->set;
    if (desc->get)current->get = desc->get;
    if (desc->value)current->value = desc->value;
    if (desc->writable)current->writable = desc->writable;
    if (desc->configurable)current->configurable = desc->configurable;
    if (desc->enumerable)current->enumerable = desc->enumerable;
    return true;
}

