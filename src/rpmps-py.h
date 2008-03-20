#ifndef H_RPMPS_PY
#define H_RPMPS_PY

#include <Python.h>

#include <rpm/rpmps.h>

/** \ingroup py_c
 * \file python/rpmps-py.h
 */

/**
 */
typedef struct rpmpsObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
    rpmps	ps;
    rpmpsi	psi;
} rpmpsObject;

/**
 */
extern PyTypeObject rpmps_Type;

/**
 */
rpmps psFromPs(rpmpsObject * ps);

/**
 */
rpmpsObject * rpmps_Wrap(rpmps ps);

#endif
