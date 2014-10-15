#include "js/object.h"

bool nodoka_isDataDescriptor(nodoka_prop_desc *desc) {
    if (!desc)return false;
    return desc->value && desc->writable;
}

bool nodoka_isAccessorDescriptor(nodoka_prop_desc *desc) {
    if (!desc)return false;
    return desc->get && desc->set;
}

bool nodoka_isGenericDescriptor(nodoka_prop_desc *desc) {
    if (!desc)return false;
    return !nodoka_isDataDescriptor(desc) && !nodoka_isAccessorDescriptor(desc);
}


