/** \ingroup py_c
 * \file python/rpmds-py.c
 */

#include <rpm/rpmtag.h>
#include <rpm/rpmstring.h>
#include <rpm/rpmlib.h>		/* rpmvercmp */

#include "header-py.h"
#include "rpmds-py.h"
#include "rpmdebug-py.h"

/**
 * Split EVR into epoch, version, and release components.
 * @param evr		[epoch:]version[-release] string
 * @retval *ep		pointer to epoch
 * @retval *vp		pointer to version
 * @retval *rp		pointer to release
 */
static
void rpmds_ParseEVR(char * evr,
		const char ** ep,
		const char ** vp,
		const char ** rp)
{
    const char *epoch;
    const char *version;		/* assume only version is present */
    const char *release;
    char *s, *se;

    s = evr;
    while (*s && risdigit(*s)) s++;	/* s points to epoch terminator */
    se = strrchr(s, '-');		/* se points to version terminator */

    if (*s == ':') {
	epoch = evr;
	*s++ = '\0';
	version = s;
	if (*epoch == '\0') epoch = "0";
    } else {
	epoch = NULL;	/* XXX disable epoch compare if missing */
	version = evr;
    }
    if (se) {
	*se++ = '\0';
	release = se;
    } else {
	release = NULL;
    }

    if (ep) *ep = epoch;
    if (vp) *vp = version;
    if (rp) *rp = release;
}

static PyObject *
rpmds_Count(rpmdsObject * s)
{
    return Py_BuildValue("i", rpmdsCount(s->ds));
}

static PyObject *
rpmds_Ix(rpmdsObject * s)
{
    return Py_BuildValue("i", rpmdsIx(s->ds));
}

static PyObject *
rpmdsDep_DNEVR(rpmdsDepObject * s)
{
    return PyString_FromString(rpmdsDNEVR(s->ds));
}

static PyObject *
rpmdsDep_N(rpmdsDepObject * s)
{
    return PyString_FromString(rpmdsN(s->ds));
}

static PyObject *
rpmdsDep_EVR(rpmdsDepObject * s)
{
    return PyString_FromString(rpmdsEVR(s->ds));
}

static PyObject *
rpmdsDep_Flags(rpmdsDepObject * s)
{
    return Py_BuildValue("i", rpmdsFlags(s->ds));
}

static PyObject *
rpmds_BT(rpmdsObject * s)
{
    return Py_BuildValue("i", (int) rpmdsBT(s->ds));
}

static PyObject *
rpmdsDep_TagN(rpmdsDepObject * s)
{
    return Py_BuildValue("i", rpmdsTagN(s->ds));
}

static PyObject *
rpmdsDep_Color(rpmdsDepObject * s)
{
    return Py_BuildValue("i", rpmdsColor(s->ds));
}

static PyObject *
rpmdsDep_Refs(rpmdsDepObject * s)
{
    return Py_BuildValue("i", rpmdsRefs(s->ds));
}

/**
 */
static int compare_values(const char *str1, const char *str2)
{
    if (!str1 && !str2)
	return 0;
    else if (str1 && !str2)
	return 1;
    else if (!str1 && str2)
	return -1;
    return rpmvercmp(str1, str2);
}

static int
rpmds_compare(rpmdsObject * a, rpmdsObject * b)
{
    char *aEVR = strdup(rpmdsEVR(a->ds));
    const char *aE, *aV, *aR;
    char *bEVR = strdup(rpmdsEVR(b->ds));
    const char *bE, *bV, *bR;
    int rc;

    /* XXX W2DO? should N be compared? */
    rpmds_ParseEVR(aEVR, &aE, &aV, &aR);
    rpmds_ParseEVR(bEVR, &bE, &bV, &bR);

    rc = compare_values(aE, bE);
    if (!rc) {
	rc = compare_values(aV, bV);
	if (!rc)
	    rc = compare_values(aR, bR);
    }

    free(aEVR);
    free(bEVR);

    return rc;
}

static PyObject *
rpmds_richcompare(rpmdsObject * a, rpmdsObject * b, int op)
{
    int rc;

    switch (op) {
    case Py_NE:
	/* XXX map ranges overlap boolean onto '!=' python syntax. */
	rc = rpmdsCompare(a->ds, b->ds);
	rc = (rc < 0 ? -1 : (rc == 0 ? 1 : 0));
	break;
    case Py_LT:
    case Py_LE:
    case Py_GT:
    case Py_GE:
    case Py_EQ:
    default:
	rc = -1;
	break;
    }
    return Py_BuildValue("i", rc);
}

static PyObject *
rpmds_iter(rpmdsObject * s)
{
    Py_INCREF(s);
    return (PyObject *)s;
}

static PyObject *
rpmds_iternext(rpmdsObject * s)
{
    PyObject * result = NULL;

    /* Reset loop indices on 1st entry. */
    if (s->cur == NULL) {
	s->ds = rpmdsInit(s->ds);
    } else {
	Py_DECREF(s->cur);
    }

    /* If more to do, return a (N, EVR, Flags) tuple. */
    if (rpmdsNext(s->ds) >= 0) {
	s->cur = PyObject_New(rpmdsDepObject, &rpmdsDep_Type);
	s->cur->ds = s->ds;
	result = (PyObject *) s->cur;
    } else {
	s->cur = NULL;
    }

    return result;
}

static PyObject *
rpmds_Next(rpmdsObject * s)
{
    PyObject * result;

    result = rpmds_iternext(s);

    if (result == NULL) {
	Py_INCREF(Py_None);
        return Py_None;
    }
    return result;
}

static PyObject *
rpmds_SetNoPromote(rpmdsObject * s, PyObject * args, PyObject * kwds)
{
    int nopromote;
    char * kwlist[] = {"noPromote", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i:SetNoPromote", kwlist,
	    &nopromote))
	return NULL;

    return Py_BuildValue("i", rpmdsSetNoPromote(s->ds, nopromote));
}

static PyObject *
rpmds_Notify(rpmdsObject * s, PyObject * args, PyObject * kwds)
{
    const char * where;
    int rc;
    char * kwlist[] = {"location", "returnCode", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "si:Notify", kwlist,
	    &where, &rc))
	return NULL;

    rpmdsNotify(s->ds, where, rc);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rpmds_Find(rpmdsObject * s, PyObject * args, PyObject * kwds)
{
    PyObject * to = NULL;
    rpmdsObject * o;
    int rc;
    char * kwlist[] = {"element", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O:Find", kwlist, &to))
	return NULL;

    /* XXX ds type check needed. */
    o = (rpmdsObject *)to;

    /* XXX make sure ods index is valid, real fix in lib/rpmds.c. */
    if (rpmdsIx(o->ds) == -1)	rpmdsSetIx(o->ds, 0);

    rc = rpmdsFind(s->ds, o->ds);
    return Py_BuildValue("i", rc);
}

static PyObject *
rpmds_Merge(rpmdsObject * s, PyObject * args, PyObject * kwds)
{
    PyObject * to = NULL;
    rpmdsObject * o;
    char * kwlist[] = {"element", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O:Merge", kwlist, &to))
	return NULL;

    /* XXX ds type check needed. */
    o = (rpmdsObject *)to;
    return Py_BuildValue("i", rpmdsMerge(&s->ds, o->ds));
}
static PyObject *
rpmds_Search(rpmdsObject * s, PyObject * args, PyObject * kwds)
{
    PyObject * to = NULL;
    rpmdsObject * o;
    char * kwlist[] = {"element", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O:Merge", kwlist, &to))
        return NULL;

    /* XXX ds type check needed. */
    o = (rpmdsObject *)to;
    return Py_BuildValue("i", rpmdsSearch(s->ds, o->ds));
}

static PyObject *
rpmds_Rpmlib(rpmdsObject * s)
{
    rpmds ds = NULL;
    int xx;

    /* XXX check return code, permit arg (NULL uses system default). */
    xx = rpmdsRpmlib(&ds, NULL);

    return (PyObject *) rpmds_Wrap( ds );
}

static struct PyMethodDef rpmdsDep_methods[] = {
 {"DNEVR",	(PyCFunction)rpmdsDep_DNEVR,	METH_NOARGS,
	"ds.DNEVR -> DNEVR	- Return current DNEVR.\n" },
 {"N",		(PyCFunction)rpmdsDep_N,		METH_NOARGS,
	"ds.N -> N		- Return current N.\n" },
 {"EVR",	(PyCFunction)rpmdsDep_EVR,		METH_NOARGS,
	"ds.EVR -> EVR		- Return current EVR.\n" },
 {"Flags",	(PyCFunction)rpmdsDep_Flags,	METH_NOARGS,
	"ds.Flags -> Flags	- Return current Flags.\n" },
 {"TagN",	(PyCFunction)rpmdsDep_TagN,	METH_NOARGS,
	"ds.TagN -> TagN	- Return current TagN.\n" },
 {"Color",	(PyCFunction)rpmdsDep_Color,	METH_NOARGS,
	"ds.Color -> Color	- Return current Color.\n" },
 {"Refs",	(PyCFunction)rpmdsDep_Refs,	METH_NOARGS,
	"ds.Refs -> Refs	- Return current Refs.\n" },
 {NULL,		NULL}		/* sentinel */
};

static struct PyMethodDef rpmds_methods[] = {
 {"Count",	(PyCFunction)rpmds_Count,	METH_NOARGS,
	"ds.Count -> Count	- Return no. of elements.\n" },
 {"Ix",		(PyCFunction)rpmds_Ix,		METH_NOARGS,
	"ds.Ix -> Ix		- Return current element index.\n" },
 {"BT",		(PyCFunction)rpmds_BT,		METH_NOARGS,
	"ds.BT -> BT	- Return build time.\n" },
 {"next",	(PyCFunction)rpmds_Next,	METH_NOARGS,
"ds.next() -> (N, EVR, Flags)\n\
- Retrieve next dependency triple.\n" },
 {"SetNoPromote",(PyCFunction)rpmds_SetNoPromote, METH_VARARGS|METH_KEYWORDS,
	NULL},
 {"Notify",	(PyCFunction)rpmds_Notify,	METH_VARARGS|METH_KEYWORDS,
	NULL},
 {"Find",	(PyCFunction)rpmds_Find,	METH_VARARGS|METH_KEYWORDS,
	NULL},
 {"Merge",	(PyCFunction)rpmds_Merge,	METH_VARARGS|METH_KEYWORDS,
	NULL},
 {"Search",     (PyCFunction)rpmds_Search,      METH_VARARGS|METH_KEYWORDS,
"ds.Search(element) -> matching ds index (-1 on failure)\n\
- Check that element dependency range overlaps some member of ds.\n\
The current index in ds is positioned at overlapping member upon success.\n" },
 {"Rpmlib",     (PyCFunction)rpmds_Rpmlib,      METH_NOARGS|METH_STATIC,
	"ds.Rpmlib -> nds       - Return internal rpmlib dependency set.\n"},
 {NULL,		NULL}		/* sentinel */
};

/* ---------- */

static void
rpmds_dealloc(rpmdsObject * s)
{
    if (s) {
	s->ds = rpmdsFree(s->ds);
	PyObject_Del(s);
    }
}

static int
rpmds_print(rpmdsObject * s, FILE * fp, int flags)
{
    if (!(s && s->ds))
	return -1;

    s->ds = rpmdsInit(s->ds);
    while (rpmdsNext(s->ds) >= 0)
	fprintf(fp, "%s\n", rpmdsDNEVR(s->ds));
    return 0;
}

static int
rpmds_length(rpmdsObject * s)
{
    return rpmdsCount(s->ds);
}

static PyObject *
rpmds_subscript(rpmdsObject * s, PyObject * key)
{
    int ix;

    if (!PyInt_Check(key)) {
	PyErr_SetString(PyExc_TypeError, "integer expected");
	return NULL;
    }

    ix = (int) PyInt_AsLong(key);
    /* XXX make sure that DNEVR exists. */
    rpmdsSetIx(s->ds, ix-1);
    (void) rpmdsNext(s->ds);
    return PyString_FromString(rpmdsDNEVR(s->ds));
}

static PyMappingMethods rpmds_as_mapping = {
        (lenfunc) rpmds_length,		/* mp_length */
        (binaryfunc) rpmds_subscript,	/* mp_subscript */
        (objobjargproc)0,		/* mp_ass_subscript */
};

/** \ingroup py_c
 */
static int rpmds_init(rpmdsObject * s, PyObject *args, PyObject *kwds)
{
    hdrObject * ho = NULL;
    PyObject * to = NULL;
    rpmTag tagN = RPMTAG_REQUIRENAME;
    rpmsenseFlags flags = 0;
    char * kwlist[] = {"header", "tag", "flags", NULL};

    debug("(%p,%p,%p)\n", s, args, kwds);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|Oi:rpmds_init", kwlist, 
	    &hdr_Type, &ho, &to, &flags))
	return -1;

    if (to != NULL) {
	tagN = tagNumFromPyObject(to);
	if (tagN == -1) {
	    PyErr_SetString(PyExc_KeyError, "unknown header tag");
	    return -1;
	}
    }
    s->ds = rpmdsNew(hdrGetHeader(ho), tagN, flags);
    s->cur = NULL;

    return 0;
}

/** \ingroup py_c
 */
static void rpmds_free(rpmdsObject * s)
{
    debug("%p -- ds %p\n", s, s->ds);
    s->ds = rpmdsFree(s->ds);

    PyObject_Del((PyObject *)s);
}

/** \ingroup py_c
 */
static PyObject * rpmds_new(PyTypeObject * subtype, PyObject *args, PyObject *kwds)
{
    rpmdsObject * s = (void *) PyObject_New(rpmdsObject, subtype);

    /* Perform additional initialization. */
    if (rpmds_init(s, args, kwds) < 0) {
	rpmds_free(s);
	return NULL;
    }

    debug("%p ++ ds %p\n", s, s->ds);

    return (PyObject *)s;
}

/**
 */
static char rpmds_doc[] =
"";

static void rpmdsDep_dealloc(rpmdsObject * s)
{
    debug("(%p)\n", s);

    PyObject_Del((PyObject *) s);
}

static char rpmdsDep_doc[] =
"";

PyTypeObject rpmds_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpmds",			/* tp_name */
	sizeof(rpmdsObject),		/* tp_basicsize */
	0,				/* tp_itemsize */
	/* methods */
	(destructor) rpmds_dealloc,	/* tp_dealloc */
	(printfunc) rpmds_print,	/* tp_print */
	(getattrfunc)0,			/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	(cmpfunc) rpmds_compare,	/* tp_compare */
	(reprfunc)0,			/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	&rpmds_as_mapping,		/* tp_as_mapping */
	(hashfunc)0,			/* tp_hash */
	(ternaryfunc)0,			/* tp_call */
	(reprfunc)0,			/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |		/* tp_flags */
	    Py_TPFLAGS_HAVE_RICHCOMPARE,
	rpmds_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	(richcmpfunc) rpmds_richcompare,/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	(getiterfunc) rpmds_iter,	/* tp_iter */
	(iternextfunc) rpmds_iternext,	/* tp_iternext */
	rpmds_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc) rpmds_init,		/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc) rpmds_new,		/* tp_new */
	(freefunc) rpmds_free,		/* tp_free */
	0,				/* tp_is_gc */
};

PyTypeObject rpmdsDep_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"dsDep",			/* tp_name */
	sizeof(rpmdsDepObject),		/* tp_basicsize */
	0,				/* tp_itemsize */
	/* methods */
	(destructor) rpmdsDep_dealloc,	/* tp_dealloc */
	(printfunc)0,			/* tp_print */
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
	Py_TPFLAGS_DEFAULT, 		/* tp_flags */
	rpmdsDep_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	(richcmpfunc)0,			/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	(getiterfunc)0,			/* tp_iter */
	(iternextfunc)0,		/* tp_iternext */
	rpmdsDep_methods,		/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc)0,			/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc)0,			/* tp_new */
	(freefunc)0,			/* tp_free */
	0,				/* tp_is_gc */
};
/* ---------- */

rpmds dsFromDs(rpmdsObject * s)
{
    return s->ds;
}

rpmdsObject *
rpmds_Wrap(rpmds ds)
{
    rpmdsObject * s = PyObject_New(rpmdsObject, &rpmds_Type);

    if (s == NULL)
	return NULL;
    s->ds = ds;
    s->cur = NULL;
    return s;
}

rpmdsObject *
rpmds_Single(PyObject * s, PyObject * args, PyObject * kwds)
{
    PyObject * to = NULL;
    rpmTag tagN = RPMTAG_PROVIDENAME;
    const char * N;
    const char * EVR = NULL;
    rpmsenseFlags Flags = 0;
    char * kwlist[] = {"to", "name", "evr", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os|si:Single", kwlist,
	    &to, &N, &EVR, &Flags))
	return NULL;

    if (to != NULL) {
	tagN = tagNumFromPyObject(to);
	if (tagN == -1) {
	    PyErr_SetString(PyExc_KeyError, "unknown header tag");
	    return NULL;
	}
    }
    if (N != NULL) N = strdup(N);
    if (EVR != NULL) EVR = strdup(EVR);
    return rpmds_Wrap( rpmdsSingle(tagN, N, EVR, Flags) );
}

rpmdsObject *
hdr_dsFromHeader(PyObject * s, PyObject * args, PyObject * kwds)
{
    hdrObject * ho = (hdrObject *)s;
    PyObject * to = NULL;
    rpmTag tagN = RPMTAG_REQUIRENAME;
    rpmsenseFlags flags = 0;
    char * kwlist[] = {"to", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi:dsFromHeader", kwlist,
	    &to, &flags))
	return NULL;

    if (to != NULL) {
	tagN = tagNumFromPyObject(to);
	if (tagN == -1) {
	    PyErr_SetString(PyExc_KeyError, "unknown header tag");
	    return NULL;
	}
    }
    return rpmds_Wrap( rpmdsNew(hdrGetHeader(ho), tagN, flags) );
}

rpmdsObject *
hdr_dsOfHeader(PyObject * s)
{
    hdrObject * ho = (hdrObject *)s;
    rpmTag tagN = RPMTAG_PROVIDENAME;
    rpmsenseFlags Flags = RPMSENSE_EQUAL;

    return rpmds_Wrap( rpmdsThis(hdrGetHeader(ho), tagN, Flags) );
}
