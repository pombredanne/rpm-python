#ifndef _RPMFI_PY_H
#define _RPMFI_PY_H

#include <Python.h>

#include <rpm/rpmfi.h>

/** \ingroup py_c
 * \file python/rpmfi-py.h
 */

typedef struct rpmfiFileObject_s {
    PyObject_HEAD
    rpmfi fi;
} rpmfiFileObject;

typedef struct rpmfiObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
    rpmfiFileObject * cur;
    rpmfi fi;
} rpmfiObject;

extern PyTypeObject rpmfi_Type;

extern PyTypeObject rpmfiFile_Type;

rpmfi fiFromFi(rpmfiObject * fi);

PyObject * rpmfi_Wrap(rpmfi fi);

#endif
