#ifndef _RPMHEADER_PY_H
#define _RPMHEADER_PY_H

#include <Python.h>

#include <rpm/rpmtypes.h>

/** \ingroup py_c
 * \file python/header-py.h
 */

/** \ingroup py_c
 */
typedef struct hdrObject_s hdrObject;

extern PyTypeObject hdr_Type;

/** \ingroup py_c
 */
extern PyObject * pyrpmError;

#define hdrObject_Check(v)	((v)->ob_type == &hdr_Type)

PyObject * hdr_Wrap(Header h);

Header hdrGetHeader(hdrObject * h);

rpmTag tagNumFromPyObject (PyObject *item);

PyObject * labelCompare (PyObject * self, PyObject * args);
PyObject * versionCompare (PyObject * self, PyObject * args, PyObject * kwds);
PyObject * rpmMergeHeadersFromFD(PyObject * self, PyObject * args, PyObject * kwds);
int rpmMergeHeaders(PyObject * list, FD_t fd, int matchTag);
PyObject * rpmHeaderFromIO(PyObject * self, PyObject * args, PyObject * kwds);
PyObject * rpmSingleHeaderFromFD(PyObject * self, PyObject * args, PyObject * kwds);
PyObject * rpmReadHeaders (FD_t fd);
PyObject * hdrLoad(PyObject * self, PyObject * args, PyObject * kwds);

#endif
