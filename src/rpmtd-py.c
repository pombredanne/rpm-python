/** \ingroup py_c
 * \file python/rpmtd-py.c
 */

#include <rpm/rpmtd.h>

#include "rpmtd-py.h"
#include "rpmdebug-py.h"

/** \ingroup python
 * \class Rpmtd
 * \brief A python rpm.td tag data container object represents header / 
 *        extension tag data.
 *
 */

/** \ingroup python
 * \name Class: Rpmtd
 */

static PyObject *rpmtd_iter(rpmtdObject *self)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *rpmtd_iternext(rpmtdObject *self)
{
    PyObject *next = NULL;

    if (rpmtdNext(self->td) >= 0) {
	Py_INCREF(self);
	next = (PyObject*) self;
    } 
    return next;
}

static PyObject *rpmtd_Format(rpmtdObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[] = {"fmt", NULL};
    char *str;
    rpmtdFormats fmt;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &fmt))
        return NULL;

    str = rpmtdFormat(self->td, fmt, NULL);
    if (str) {
	return PyString_FromString(str);
    }
    Py_RETURN_NONE;
}

/** \ingroup py_c
 */
static struct PyMethodDef rpmtd_methods[] = {
    {"format",	    (PyCFunction) rpmtd_Format,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {NULL,		NULL}		/* sentinel */
};

static int rpmtd_length(rpmtdObject *self)
{
    return rpmtdCount(self->td);
}

/** \ingroup py_c
 */
static void rpmtd_dealloc(rpmtdObject * s)
{
    if (s) {
	rpmtdFreeData(s->td);
	rpmtdFree(s->td);
	PyObject_Del(s);
    }
}

/**
 */
static char rpmtd_doc[] =
"";

static PyMappingMethods rpmtd_as_mapping = {
    (lenfunc) rpmtd_length,		/* mp_length */
};

/** \ingroup py_c
 */
PyTypeObject rpmtd_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.td",			/* tp_name */
	sizeof(rpmtdObject),		/* tp_size */
	0,				/* tp_itemsize */
	(destructor) rpmtd_dealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	(getattrfunc)0, 		/* tp_getattr */
	0,				/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	&rpmtd_as_mapping,		/* tp_as_mapping */
	0,				/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	rpmtd_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	(getiterfunc) rpmtd_iter,	/* tp_iter */
	(iternextfunc) rpmtd_iternext,	/* tp_iternext */
	rpmtd_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	0,				/* tp_init */
	0,				/* tp_alloc */
	0,				/* tp_new */
	0,				/* tp_free */
	0,				/* tp_is_gc */
};

rpmtdObject * rpmtd_Wrap(rpmtd td)
{
    rpmtdObject * tdo = (rpmtdObject *) PyObject_New(rpmtdObject, &rpmtd_Type);

    if (tdo) {
    	tdo->td = td;
    } else {
        PyErr_SetString(PyExc_MemoryError, "out of memory creating rpmtd");
    }
    return tdo;
}

