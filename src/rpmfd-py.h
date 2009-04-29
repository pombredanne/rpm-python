#ifndef _RPMFD_PY_H
#define _RPMTE_PY_H

#include <Python.h>
#include <rpm/rpmio.h>

FD_t rpmFdFromPyObject(PyObject *obj);

#endif
