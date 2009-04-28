/** \ingroup py_c
 * \file python/rpmps-py.c
 */

#include "rpmps-py.h"
#include "rpmdebug-py.h"

static int
rpmps_append(rpmpsObject * s, PyObject * value)
{
    char *pkgNEVR, *altNEVR, *str1;
    unsigned long ulong1;
    int ignoreProblem;
    rpmProblemType type;
    fnpyKey key;

    if (!PyArg_ParseTuple(value, "ssOiisN:rpmps value tuple",
			&pkgNEVR, &altNEVR, &key,
			&type, &ignoreProblem, &str1,
			&ulong1))
    {
    	return -1;
    }
    rpmpsAppend(s->ps, type, pkgNEVR, key, str1, NULL, altNEVR, ulong1);
    return 0;
}

static PyObject *
rpmps_iter(rpmpsObject * s)
{
    debug("(%p)\n", s);
    s->psi = rpmpsInitIterator(s->ps);
    Py_INCREF(s);
    return (PyObject *)s;
}

static PyObject *
rpmps_iternext(rpmpsObject * s)
{
    PyObject * result = NULL;

    debug("(%p) ps %p psi %p\n", s, s->ps, s->psi);

    /* Reset loop indices on 1st entry. */
    if (s->psi == NULL) {
    	s->psi = rpmpsInitIterator(s->ps);
    }

    /* If more to do, return a problem set string. */
    if (rpmpsNextIterator(s->psi) >= 0) {
	char * ps = rpmProblemString(rpmpsGetProblem(s->psi));
	result = PyString_FromString(ps);
	free(ps);
    } else {
	s->psi = rpmpsFreeIterator(s->psi);
    }

    return result;
}

static struct PyMethodDef rpmps_methods[] = {
  {"append",	(PyCFunction)rpmps_append,	METH_VARARGS, NULL},
 {NULL,		NULL}		/* sentinel */
};

/* ---------- */

static void
rpmps_dealloc(rpmpsObject * s)
{
    debug("(%p)\n", s);
    if (s) {
	s->ps = rpmpsFree(s->ps);
	PyObject_Del(s);
    }
}

static int
rpmps_print(rpmpsObject * s, FILE * fp, int flags)
{
    debug("(%p,%p,%x)\n", s, (void *)fp, flags);
    if (s && s->ps)
	rpmpsPrint(fp, s->ps);
    return 0;
}

static int
rpmps_length(rpmpsObject * s)
{
    int rc;
    rc = rpmpsNumProblems(s->ps);
    debug("(%p) rc %d\n", s, rc);
    return rc;
}

static PyObject *
rpmps_subscript(rpmpsObject * s, PyObject * key)
{
    PyObject * result = NULL;
    rpmpsi psi;
    int ix, i;

    if (!PyInt_Check(key)) {
	PyErr_SetString(PyExc_TypeError, "integer expected");
	return NULL;
    }

    ix = (int) PyInt_AsLong(key);
    /* XXX range check */

    psi = rpmpsInitIterator(s->ps);
    while ((i = rpmpsNextIterator(psi)) >= 0) {
	if (i == ix) {
	    char * ps = rpmProblemString(rpmpsGetProblem(psi));
	    result = PyString_FromString(ps);
	    free(ps);
	    break;
	}
    }
    psi = rpmpsFreeIterator(psi);

    debug("(%p,%p) %s\n", s, key, PyString_AsString(result));

    return result;
}

static PyMappingMethods rpmps_as_mapping = {
        (lenfunc) rpmps_length,		/* mp_length */
        (binaryfunc) rpmps_subscript,	/* mp_subscript */
};

/** \ingroup py_c
 */
static int rpmps_init(rpmpsObject * s, PyObject *args, PyObject *kwds)
{
    return 0;
}

/** \ingroup py_c
 */
static void rpmps_free(rpmpsObject * s)
{
    debug("%p -- ps %p\n", s, s->ps);
    s->ps = rpmpsFree(s->ps);

    PyObject_Del((PyObject *)s);
}

/** \ingroup py_c
 */
static PyObject * rpmps_new(PyTypeObject * subtype, PyObject *args, PyObject *kwds)
{
    rpmpsObject * s = NULL;
    char * kwlist[] = {NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, ":rpmps_init", kwlist))
	return NULL;

    s = PyObject_New(rpmpsObject, subtype);
    s->ps = rpmpsCreate();
    s->psi = NULL;

    debug("%p ++ ps %p\n", s, s->ps);

    return (PyObject *)s;
}

/**
 */
static char rpmps_doc[] =
"";

PyTypeObject rpmps_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.ps",			/* tp_name */
	sizeof(rpmpsObject),		/* tp_basicsize */
	0,				/* tp_itemsize */
	/* methods */
	(destructor) rpmps_dealloc,	/* tp_dealloc */
	(printfunc) rpmps_print,	/* tp_print */
	(getattrfunc)0,			/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	(cmpfunc)0,			/* tp_compare */
	(reprfunc)0,			/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	&rpmps_as_mapping,		/* tp_as_mapping */
	(hashfunc)0,			/* tp_hash */
	(ternaryfunc)0,			/* tp_call */
	(reprfunc)0,			/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT, 		/* tp_flags */
	rpmps_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	(richcmpfunc)0,			/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	(getiterfunc) rpmps_iter,	/* tp_iter */
	(iternextfunc) rpmps_iternext,	/* tp_iternext */
	rpmps_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc) rpmps_init,		/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc) rpmps_new,		/* tp_new */
	(freefunc) rpmps_free,		/* tp_free */
	0,				/* tp_is_gc */
};

/* ---------- */

rpmps psFromPs(rpmpsObject * s)
{
    return s->ps;
}

rpmpsObject *
rpmps_Wrap(rpmps ps)
{
    rpmpsObject * s = PyObject_New(rpmpsObject, &rpmps_Type);

    if (s == NULL) {
	return PyErr_NoMemory();
    }
    s->ps = ps;
    s->psi = NULL;
    return s;
}
