#ifndef _RPMDS_PY_H
#define _RPMDS_PY_H

#include <Python.h>

#include <rpm/rpmds.h>

/** \ingroup py_c
 * \file python/rpmds-py.h
 */

typedef struct rpmdsDepObject_s {
    PyObject_HEAD
    rpmds ds;
} rpmdsDepObject;

/**
 */
typedef struct rpmdsObject_s {
    PyObject_HEAD
    rpmdsDepObject * cur;
    rpmds ds;
} rpmdsObject;

/**
 */
extern PyTypeObject rpmds_Type;

extern PyTypeObject rpmdsDep_Type;
/**
 */
rpmds dsFromDs(rpmdsObject * ds);

/**
 */
PyObject * rpmds_Wrap(rpmds ds);

#endif
