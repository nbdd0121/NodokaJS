#include "c/stdlib.h"

#include "js/bytecode.h"
#include "js/object.h"

nodoka_envRec *nodoka_newDeclEnvRecord(nodoka_envRec *outer) {
    nodoka_envRec *rec = (nodoka_envRec *)nodoka_new_data(NODOKA_ENV);
    rec->outer = outer;
    rec->object = nodoka_newNativeObject();
    rec->this = nodoka_undefined;
    return rec;
}

nodoka_envRec *nodoka_newObjEnvRecord(nodoka_object *obj, nodoka_envRec *outer) {
    nodoka_envRec *rec = (nodoka_envRec *)nodoka_new_data(NODOKA_ENV);
    rec->outer = outer;
    rec->object = obj;
    rec->this = (nodoka_data *)obj;
    return rec;
}

bool nodoka_hasBinding(nodoka_envRec *env, nodoka_string *name) {
    return nodoka_hasProperty(env->object, name);
}

nodoka_data *nodoka_getBindingValue(nodoka_envRec *env, nodoka_string *name) {
    if (!nodoka_hasProperty(env->object, name)) {
        //strict?
        return nodoka_undefined;
    }
    return nodoka_get(env->object, name);
}

void nodoka_setMutableBinding(nodoka_envRec *env, nodoka_string *name, nodoka_data *val) {
    /* SetMutableBinding on env records will result in TypeError
     * if trying to modify immuntable bindings in strict mode */
    nodoka_put(env->object, name, val, false);
}

