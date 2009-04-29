/** \ingroup py_c
 * \file rpmlog-py.c
 */

#include <rpm/rpmlog.h>

#include "rpmlog-py.h"
#include "rpmdebug-py.h"

extern PyObject * pyrpmError;

static PyObject * rpmlog_Log(PyObject * self, PyObject * args, PyObject * kwds)
{
    char * msg;
    rpmlogLvl level;
    static char * kwlist[] = {"level", "msg", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Is", kwlist, &level, &msg))
	return NULL;

    /* XXX TODO check for arg sanity */

    rpmlog(level, "%s", msg);
    Py_RETURN_NONE;
}

/**
 */
static PyObject * rpmlog_setFile (PyObject * self, PyObject * args, PyObject *kwds)
{
    PyObject * fop = NULL;
    FILE * fp = NULL;
    char * kwlist[] = {"fileObject", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O:logSetFile", kwlist, &fop))
	return NULL;

    if (fop) {
	if (!PyFile_Check(fop)) {
	    PyErr_SetString(TypeError, "file object expected");
	    return NULL;
	}
	fp = PyFile_AsFile(fop);
    }

    (void) rpmlogSetFile(fp);

    Py_INCREF(Py_None);
    return (PyObject *) Py_None;
}

/**
 */
static PyObject *
rpmlog_setVerbosity (PyObject * self, PyObject * args, PyObject *kwds)
{
    int level;
    char * kwlist[] = {"level", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &level))
	return NULL;

    rpmSetVerbosity(level);

    Py_INCREF(Py_None);
    return (PyObject *) Py_None;
}

static struct PyMethodDef rpmlog_methods[] = {
    { "log",	    (PyCFunction) rpmlog_Log,	
	METH_VARARGS|METH_KEYWORDS, NULL },
    { "setVerbosity", (PyCFunction) rpmlog_setVerbosity, 
	METH_VARARGS|METH_KEYWORDS, NULL },
    { "setFile", (PyCFunction) rpmlog_setFile, 
	METH_VARARGS|METH_KEYWORDS, NULL },
    {NULL,		NULL}		/* sentinel */
};

static void rpmlog_dealloc(rpmlogObject * s)
{
    if (s) {
	PyObject_Del(s);
    }
}


static char rpmlog_doc[] =
"";

PyTypeObject rpmlog_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.log",			/* tp_name */
	sizeof(rpmlogObject),		/* tp_size */
	0,				/* tp_itemsize */
	(destructor) rpmlog_dealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	(getattrfunc)0, 		/* tp_getattr */
	0,				/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	0,				/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	rpmlog_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	(getiterfunc)0,			/* tp_iter */
	(iternextfunc)0,		/* tp_iternext */
	rpmlog_methods,			/* tp_methods */
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

