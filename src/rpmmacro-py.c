/** \ingroup py_c
 * \file python/rpmmacro-py.c
 */

#include <Python.h>
#include <structmember.h>

#include <rpm/rpmmacro.h>

#include "rpmmacro-py.h"
#include "rpmdebug-py.h"

/**
 */
PyObject *
rpmmacro_AddMacro(PyObject * self, PyObject * args, PyObject * kwds)
{
    char * name, * val;
    char * kwlist[] = {"name", "value", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss:AddMacro", kwlist,
	    &name, &val))
	return NULL;

    addMacro(NULL, name, NULL, val, -1);

    Py_RETURN_NONE;
}

/**
 */
PyObject *
rpmmacro_DelMacro(PyObject * self, PyObject * args, PyObject * kwds)
{
    char * name;
    char * kwlist[] = {"name", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s:DelMacro", kwlist, &name))
	return NULL;

    delMacro(NULL, name);

    Py_RETURN_NONE;
}

/**
 */
PyObject * 
rpmmacro_ExpandMacro(PyObject * self, PyObject * args, PyObject * kwds)
{
    char * macro, *buf;
    PyObject *res;

    if (!PyArg_ParseTuple(args, "s", &macro))
        return NULL;

    buf = rpmExpand(macro, NULL);
    res = PyString_FromString(buf);
    free(buf);
    return res;
}

