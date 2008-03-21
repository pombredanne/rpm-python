#ifndef H_RPMFI_PY
#define H_RPMFI_PY

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

rpmfiObject * rpmfi_Wrap(rpmfi fi);

rpmfiObject * hdr_fiFromHeader(PyObject * s, PyObject * args, PyObject * kwds);

#endif
