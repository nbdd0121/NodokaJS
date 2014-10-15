#include "c/stdlib.h"

#include "js/bytecode.h"
#include "js/object.h"

nodoka_envRec *nodoka_newDeclEnvRecord(nodoka_envRec *outer) {
    nodoka_envRec *rec = malloc(sizeof(nodoka_envRec));
    rec->outer = outer;
    rec->object = nodoka_newObject();
    rec->this = nodoka_undefined;
    return rec;
}

nodoka_envRec *nodoka_newObjEnvRecord(nodoka_object *obj, nodoka_envRec *outer) {
    nodoka_envRec *rec = malloc(sizeof(nodoka_envRec));
    rec->outer = outer;
    rec->object = obj;
    rec->this = (nodoka_data *)obj;
    return rec;
}
