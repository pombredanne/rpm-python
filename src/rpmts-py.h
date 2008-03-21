#ifndef _RPMTS_PY_H
#define _RPMTS_PY_H

#include <Python.h>

#include <rpm/rpmts.h>

/** \ingroup py_c
 * \file python/rpmts-py.h
 */

typedef struct rpmtsObject_s {
    PyObject_HEAD
    PyObject *md_dict;		/*!< to look like PyModuleObject */
    rpmts	ts;
    PyObject * keyList;		/* keeps reference counts correct */
    FD_t scriptFd;
    rpmtsi tsi;
    rpmElementType tsiFilter;
    rpmprobFilterFlags ignoreSet;
} rpmtsObject;

extern PyTypeObject rpmts_Type;

/* XXX These names/constants have been removed from the rpmlib API. */
enum {
   RPMDEP_SENSE_REQUIRES,		/*!< requirement not satisfied. */
   RPMDEP_SENSE_CONFLICTS		/*!< conflict was found. */
};

PyObject * rpmts_Create(PyObject * s, PyObject * args, PyObject * kwds);

#endif
