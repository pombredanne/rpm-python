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
    return Py_BuildValue("s", strdup(rpmfiBN(s->fi)));
}

static PyObject *
rpmfiFile_DN(rpmfiFileObject * s)
{
    return Py_BuildValue("s", strdup(rpmfiDN(s->fi)));
}

static PyObject *
rpmfiFile_FN(rpmfiFileObject * s)
{
    return Py_BuildValue("s", strdup(rpmfiFN(s->fi)));
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

/* XXX rpmfiMD5 */
static PyObject *
rpmfiFile_MD5(rpmfiFileObject * s)
{
    const unsigned char * MD5;
    char fmd5[33];
    char * t;
    int i;

    MD5 = rpmfiMD5(s->fi);
    t = fmd5;
    if (MD5 != NULL)
    for (i = 0; i < 16; i++, t += 2)
	sprintf(t, "%02x", MD5[i]);
    *t = '\0';
    return Py_BuildValue("s", strdup(fmd5));
}

static PyObject *
rpmfiFile_FLink(rpmfiFileObject * s)
{
    return Py_BuildValue("s", strdup(rpmfiFLink(s->fi)));
}

static PyObject *
rpmfiFile_FSize(rpmfiFileObject * s)
{
    return Py_BuildValue("i", rpmfiFSize(s->fi));
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
    return Py_BuildValue("s", strdup(rpmfiFUser(s->fi)));
}

static PyObject *
rpmfiFile_FGroup(rpmfiFileObject * s)
{
    return Py_BuildValue("s", strdup(rpmfiFGroup(s->fi)));
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
    return Py_BuildValue("s", strdup(FClass));
}

static PyObject *
rpmfi_iter(rpmfiObject * s)
{
    Py_INCREF(s);
    return (PyObject *)s;
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
	s->cur = PyObject_New(rpmfiFileObject, &rpmfiFile_Type);
	s->cur->fi = s->fi;
	result = (PyObject *) s->cur;
    } else {
	s->cur = NULL;
    }

    return result;
}

static PyObject *
rpmfi_Next(rpmfiObject * s)
{
    PyObject * result = NULL;

    result = rpmfi_iternext(s);

    if (result == NULL) {
	Py_INCREF(Py_None);
	return Py_None;
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
 {"MD5",	(PyCFunction)rpmfiFile_MD5,		METH_NOARGS,
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
 {"next",	(PyCFunction)rpmfi_Next,	METH_NOARGS,
"fi.next() -> (FN, FSize, FMode, FMtime, FFlags, FRdev, FInode, FNlink, FState, VFlags, FUser, FGroup, FMD5))\n\
- Retrieve next file info tuple.\n" },
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
    rpmfiSetFX(s->fi, ix);
    return Py_BuildValue("s", strdup(rpmfiFN(s->fi)));
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
    hdrObject * ho = NULL;
    PyObject * to = NULL;
    rpmts ts = NULL;	/* XXX FIXME: fiFromHeader should be a ts method. */
    rpmTag tagN = RPMTAG_BASENAMES;
    int flags = 0;
    char * kwlist[] = {"header", "tag", "flags", NULL};

    debug("(%p,%p,%p)\n", s, args, kwds);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|Oi:rpmfi_init", kwlist,
	    &hdr_Type, &ho, &to, &flags))
	return -1;

    if (to != NULL) {
	tagN = tagNumFromPyObject(to);
	if (tagN == -1) {
	    PyErr_SetString(PyExc_KeyError, "unknown header tag");
	    return -1;
	}
    }
    s->fi = rpmfiNew(ts, hdrGetHeader(ho), tagN, flags);
    s->cur = NULL;

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
    rpmfiObject * s = (void *) PyObject_New(rpmfiObject, subtype);

    /* Perform additional initialization. */
    if (rpmfi_init(s, args, kwds) < 0) {
	rpmfi_free(s);
	return NULL;
    }

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
	(getiterfunc) rpmfi_iter,	/* tp_iter */
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

rpmfiObject *
rpmfi_Wrap(rpmfi fi)
{
    rpmfiObject *s = PyObject_New(rpmfiObject, &rpmfi_Type);

    if (s == NULL)
	return NULL;
    s->fi = fi;
    s->cur = NULL;
    return s;
}

rpmfiObject *
hdr_fiFromHeader(PyObject * s, PyObject * args, PyObject * kwds)
{
    hdrObject * ho = (hdrObject *)s;
    PyObject * to = NULL;
    rpmts ts = NULL;	/* XXX FIXME: fiFromHeader should be a ts method. */
    rpmTag tagN = RPMTAG_BASENAMES;
    int flags = 0;
    char * kwlist[] = {"tag", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi:fiFromHeader", kwlist,
	    &to, &flags))
	return NULL;

    if (to != NULL) {
	tagN = tagNumFromPyObject(to);
	if (tagN == -1) {
	    PyErr_SetString(PyExc_KeyError, "unknown header tag");
	    return NULL;
	}
    }
    return rpmfi_Wrap( rpmfiNew(ts, hdrGetHeader(ho), tagN, flags) );
}
