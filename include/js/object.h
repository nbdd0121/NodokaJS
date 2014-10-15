#ifndef JS_OBJECT_H
#define JS_OBJECT_H

#include "js/js.h"
#include "data-struct/hashmap.h"

struct nodoka_prop_desc {
    nodoka_data base;
    nodoka_data *value;
    nodoka_data *get;
    nodoka_data *set;
    nodoka_data *writable;
    nodoka_data *enumerable;
    nodoka_data *configurable;
};

typedef nodoka_prop_desc *(*nodoka_getOwnProperty_func)(nodoka_object *O, nodoka_string *P);
typedef nodoka_object *(*nodoka_construct_func)(nodoka_object *O, int argc, nodoka_data **argv);
typedef enum nodoka_completion (*nodoka_call_func)(nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv);


struct nodoka_object {
    nodoka_data base;
    hashmap_t *prop;
    nodoka_object *prototype;
    //nodoka_string* _class;
    bool extensible;
    nodoka_getOwnProperty_func getOwnProperty;
    nodoka_call_func call;
    nodoka_construct_func construct;
    nodoka_code *code;
};

nodoka_prop_desc *nodoka_getOwnProperty(nodoka_object *O, nodoka_string *P);
nodoka_prop_desc *nodoka_getProperty(nodoka_object *O, nodoka_string *P);
nodoka_data *nodoka_get(nodoka_object *O, nodoka_string *P);
bool nodoka_canPut(nodoka_object *O, nodoka_string *P);
void nodoka_put(nodoka_object *O, nodoka_string *P, nodoka_data *V, bool throw);
bool nodoka_hasOwnProperty(nodoka_object *O, nodoka_string *P);
bool nodoka_delete(nodoka_object *O, nodoka_string *P, bool throw);
nodoka_data *nodoka_defaultValue(nodoka_object *O, enum nodoka_data_type hint);
bool nodoka_defineOwnProperty(nodoka_object *O, nodoka_string *P, nodoka_prop_desc *desc, bool throw);

enum nodoka_completion nodoka_call(nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv);
nodoka_object *nodoka_construct(nodoka_object *O, int argc, nodoka_data **argv);


/* prop.c */
bool nodoka_isDataDescriptor(nodoka_prop_desc *desc);
bool nodoka_isAccessorDescriptor(nodoka_prop_desc *desc);
bool nodoka_isGenericDescriptor(nodoka_prop_desc *desc);

nodoka_prop_desc *nodoka_newPropertyDesc(void);
nodoka_object *nodoka_newObject(void);

#endif