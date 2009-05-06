/** \ingroup py_c
 * \file python/rpmfi-py.c
 */

#include <rpm/rpmtag.h>

#include "header-py.h"
#include "rpmfi-py.h"
#include "rpmdebug-py.h"

static PyObject *
rpmfi_FC(rpmfiObject * s)
{
    return Py_BuildValue("i", rpmfiFC(s->fi));
}

static PyObject *
rpmfi_FX(rpmfiObject * s)
{
    return Py_BuildValue("i", rpmfiFX(s->fi));
}

static PyObject *
rpmfi_DC(rpmfiObject * s)
{
    return Py_BuildValue("i", rpmfiDC(s->fi));
}

static PyObject *
rpmfi_DX(rpmfiObject * s)
{
    return Py_BuildValue("i", rpmfiDX(s->fi));
}

static PyObject *
rpmfiFile_BN(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiBN(s->fi));
}

static PyObject *
rpmfiFile_DN(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiDN(s->fi));
}

static PyObject *
rpmfiFile_FN(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiFN(s->fi));
}

static PyObject *
rpmfiFile_FFlags(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFFlags(s->fi));
}

static PyObject *
rpmfiFile_VFlags(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiVFlags(s->fi));
}

static PyObject *
rpmfiFile_FMode(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFMode(s->fi));
}

static PyObject *
rpmfiFile_FState(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFState(s->fi));
}

static PyObject *
rpmfiFile_Digest(rpmfiFileObject * s)
{
    char *digest = rpmfiFDigestHex(s->fi, NULL);
    if (digest) {
	PyObject *dig = PyString_FromString(digest);
	free(digest);
	return dig;
    } else {
	Py_RETURN_NONE;
    }
}

static PyObject *
rpmfiFile_FLink(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiFLink(s->fi));
}

static PyObject *
rpmfiFile_FSize(rpmfiFileObject * s)
{
    return Py_BuildValue("K", rpmfiFSize(s->fi));
}

static PyObject *
rpmfiFile_FRdev(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFRdev(s->fi));
}

static PyObject *
rpmfiFile_FMtime(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFMtime(s->fi));
}

static PyObject *
rpmfiFile_FUser(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiFUser(s->fi));
}

static PyObject *
rpmfiFile_FGroup(rpmfiFileObject * s)
{
    return PyString_FromString(rpmfiFGroup(s->fi));
}

static PyObject *
rpmfiFile_FColor(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFColor(s->fi));
}

static PyObject *
rpmfiFile_FClass(rpmfiFileObject * s)
{
    const char * FClass;

    if ((FClass = rpmfiFClass(s->fi)) == NULL)
	FClass = "";
    return PyString_FromString(FClass);
}

static rpmfiFileObject *
rpmfiFile_Wrap(rpmfi fi)
{
    rpmfiFileObject *fo = PyObject_New(rpmfiFileObject, &rpmfiFile_Type);
    if (fo == NULL) {
	return PyErr_NoMemory();
    }
    fo->fi = fi;
    return fo;
}

static PyObject *
rpmfi_iternext(rpmfiObject * s)
{
    PyObject * result = NULL;

    /* Reset loop indices on 1st entry. */
    if (s->cur == NULL) {
	s->fi = rpmfiInit(s->fi, 0);
    } else {
	Py_DECREF(s->cur);
    }

    /* If more to do, return a new rpmfiFile object. */
    if (rpmfiNext(s->fi) >= 0) {
	s->cur = rpmfiFile_Wrap(s->fi);
	result = (PyObject *) s->cur;
    } else {
	s->cur = NULL;
    }

    return result;
}

static struct PyMethodDef rpmfiFile_methods[] = {
 {"BN",		(PyCFunction)rpmfiFile_BN,		METH_NOARGS,
	NULL},
 {"DN",		(PyCFunction)rpmfiFile_DN,		METH_NOARGS,
	NULL},
 {"FN",		(PyCFunction)rpmfiFile_FN,		METH_NOARGS,
	NULL},
 {"FFlags",	(PyCFunction)rpmfiFile_FFlags,	METH_NOARGS,
	NULL},
 {"VFlags",	(PyCFunction)rpmfiFile_VFlags,	METH_NOARGS,
	NULL},
 {"FMode",	(PyCFunction)rpmfiFile_FMode,	METH_NOARGS,
	NULL},
 {"FState",	(PyCFunction)rpmfiFile_FState,	METH_NOARGS,
	NULL},
 {"Digest",	(PyCFunction)rpmfiFile_Digest,		METH_NOARGS,
	NULL},
 {"FLink",	(PyCFunction)rpmfiFile_FLink,	METH_NOARGS,
	NULL},
 {"FSize",	(PyCFunction)rpmfiFile_FSize,	METH_NOARGS,
	NULL},
 {"FRdev",	(PyCFunction)rpmfiFile_FRdev,	METH_NOARGS,
	NULL},
 {"FMtime",	(PyCFunction)rpmfiFile_FMtime,	METH_NOARGS,
	NULL},
 {"FUser",	(PyCFunction)rpmfiFile_FUser,	METH_NOARGS,
	NULL},
 {"FGroup",	(PyCFunction)rpmfiFile_FGroup,	METH_NOARGS,
	NULL},
 {"FColor",	(PyCFunction)rpmfiFile_FColor,	METH_NOARGS,
	NULL},
 {"FClass",	(PyCFunction)rpmfiFile_FClass,	METH_NOARGS,
	NULL},
 {NULL,		NULL}		/* sentinel */
};

static struct PyMethodDef rpmfi_methods[] = {
 {"FC",		(PyCFunction)rpmfi_FC,		METH_NOARGS,
	NULL},
 {"FX",		(PyCFunction)rpmfi_FX,		METH_NOARGS,
	NULL},
 {"DC",		(PyCFunction)rpmfi_DC,		METH_NOARGS,
	NULL},
 {"DX",		(PyCFunction)rpmfi_DX,		METH_NOARGS,
	NULL},
 {NULL,		NULL}		/* sentinel */
};

/* ---------- */

static void
rpmfi_dealloc(rpmfiObject * s)
{
    if (s) {
	s->fi = rpmfiFree(s->fi);
	PyObject_Del(s);
    }
}

static int
rpmfi_print(rpmfiObject * s, FILE * fp, int flags)
{
    if (!(s && s->fi))
	return -1;

    s->fi = rpmfiInit(s->fi, 0);
    while (rpmfiNext(s->fi) >= 0)
	fprintf(fp, "%s\n", rpmfiFN(s->fi));
    return 0;
}

static int
rpmfi_length(rpmfiObject * s)
{
    return rpmfiFC(s->fi);
}

static PyObject *
rpmfi_subscript(rpmfiObject * s, PyObject * key)
{
    int ix;

    if (!PyInt_Check(key)) {
	PyErr_SetString(PyExc_TypeError, "integer expected");
	return NULL;
    }

    ix = (int) PyInt_AsLong(key);
    /* manual bounds check as rpmfiSetFX() can return -1 on non-errors, ugh */
    if (ix >= 0 && ix < rpmfiFC(s->fi)) {
	rpmfiSetFX(s->fi, ix);
	return (PyObject *) rpmfiFile_Wrap(s->fi);
    } else {
	PyErr_SetString(PyExc_IndexError, "index out of bounds");
	return NULL;
    }
}

static PyMappingMethods rpmfi_as_mapping = {
        (lenfunc) rpmfi_length,		/* mp_length */
        (binaryfunc) rpmfi_subscript,	/* mp_subscript */
        (objobjargproc)0,		/* mp_ass_subscript */
};

/** \ingroup py_c
 */
static int rpmfi_init(rpmfiObject * s, PyObject *args, PyObject *kwds)
{
    return 0;
}

/** \ingroup py_c
 */
static void rpmfi_free(rpmfiObject * s)
{
    debug("%p -- fi %p\n", s, s->fi);
    s->fi = rpmfiFree(s->fi);

    PyObject_Del((PyObject *)s);
}

/** \ingroup py_c
 */
static PyObject * rpmfi_new(PyTypeObject * subtype, PyObject *args, PyObject *kwds)
{
    hdrObject * ho = NULL;
    rpmfiObject *s = NULL;
    char * kwlist[] = {"header", "flags", NULL};
    rpmfiFlags flags = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|i:rpmfi_init", kwlist,
	    &hdr_Type, &ho, &flags))
	return NULL;

    s = PyObject_New(rpmfiObject, subtype);
    if (s == NULL) {
	return PyErr_NoMemory();
    }
    s->fi = rpmfiNew(NULL, hdrGetHeader(ho), RPMTAG_BASENAMES, flags);
    s->cur = NULL;

    debug("%p ++ fi %p\n", s, s->fi);

    return (PyObject *)s;
}

/**
 */
static char rpmfi_doc[] =
"";

PyTypeObject rpmfi_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.fi",			/* tp_name */
	sizeof(rpmfiObject),		/* tp_basicsize */
	0,				/* tp_itemsize */
	/* methods */
	(destructor) rpmfi_dealloc,	/* tp_dealloc */
	(printfunc) rpmfi_print,	/* tp_print */
	(getattrfunc)0,			/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	(cmpfunc)0,			/* tp_compare */
	(reprfunc)0,			/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	&rpmfi_as_mapping,		/* tp_as_mapping */
	(hashfunc)0,			/* tp_hash */
	(ternaryfunc)0,			/* tp_call */
	(reprfunc)0,			/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	rpmfi_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	PyObject_SelfIter,		/* tp_iter */
	(iternextfunc) rpmfi_iternext,	/* tp_iternext */
	rpmfi_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc) rpmfi_init,		/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc) rpmfi_new,		/* tp_new */
	(freefunc) rpmfi_free,		/* tp_free */
	0,				/* tp_is_gc */
};

PyTypeObject rpmfiFile_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpmfiFile",			/* tp_name */
	sizeof(rpmfiFileObject),	/* tp_basicsize */
	0,				/* tp_itemsize */
	/* methods */
	(destructor)PyObject_Del,	/* tp_dealloc */
	(printfunc) 0,			/* tp_print */
	(getattrfunc)0,			/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	(cmpfunc)0,			/* tp_compare */
	(reprfunc)0,			/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	(hashfunc)0,			/* tp_hash */
	(ternaryfunc)0,			/* tp_call */
	(reprfunc)0,			/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,				/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	rpmfiFile_methods,		/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc) 0,			/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc) 0,			/* tp_new */
	(freefunc) 0,			/* tp_free */
	0,				/* tp_is_gc */
};
/* ---------- */

rpmfi fiFromFi(rpmfiObject * s)
{
    return s->fi;
}

PyObject *
rpmfi_Wrap(rpmfi fi)
{
    rpmfiObject *s = PyObject_New(rpmfiObject, &rpmfi_Type);

    if (s == NULL) {
	return PyErr_NoMemory();
    }
    s->fi = fi;
    s->cur = NULL;
    return (PyObject *) s;
}
