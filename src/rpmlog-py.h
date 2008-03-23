#ifndef _RPMLOG_PY_H
#define _RPMLOG_PY_H

#include <Python.h>

/** \ingroup py_c
 * \file rpmlog-py.h
 */

/** \ingroup py_c
 */
typedef struct rpmlogObject_s rpmlogObject;

/** \ingroup py_c
 */
struct rpmlogObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
};

extern PyTypeObject rpmlog_Type;

#endif
