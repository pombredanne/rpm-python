#ifndef _RPMMI_PY_H
#define _RPMMI_PY_H

#include <Python.h>

/** \ingroup py_c
 * \file python/rpmmi-py.h
 */

/** \ingroup py_c
 */
typedef struct rpmmiObject_s rpmmiObject;

/** \ingroup py_c
 */
struct rpmmiObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
    PyObject *ref;		/* for db/ts refcounting */
    rpmdbMatchIterator mi;
} ;

extern PyTypeObject rpmmi_Type;

PyObject * rpmmi_Wrap(rpmdbMatchIterator mi, PyObject *s);

#endif
