#ifndef H_RPMAL_PY
#define H_RPMAL_PY

#include <Python.h>

#include <rpm/rpmal.h>

/** \ingroup py_c
 * \file python/rpmal-py.h
 */

typedef struct rpmalObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
    rpmal	al;
} rpmalObject;

extern PyTypeObject rpmal_Type;

rpmalObject * rpmal_Wrap(rpmal al);

#endif
