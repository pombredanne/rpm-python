#ifndef _RPMFD_PY_H
#define _RPMTE_PY_H

#include <Python.h>
#include <rpm/rpmio.h>

FD_t rpmFdFromPyObject(PyObject *obj);

typedef struct rpmfdObject_s {
    PyObject_HEAD
    PyObject *md_dict;
    FD_t fd;
} rpmfdObject;

extern PyTypeObject rpmfd_Type;
#endif
