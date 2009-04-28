/** \ingroup py_c
 * \file python/rpmte-py.c
 */

#include "header-py.h"	/* XXX tagNumFromPyObject */
#include "rpmds-py.h"
#include "rpmfi-py.h"
#include "rpmte-py.h"
#include "rpmdebug-py.h"

/** \ingroup python
 * \name Class: Rpmte
 * \class Rpmte
 * \brief An python rpm.te object represents an element of a transaction set.
 *
 * Elements of a transaction set are accessible after being added. Each
 * element carries descriptive information about the added element as well
 * as a file info set and dependency sets for each of the 4 types of dependency.
 *
 * The rpmte class contains the following methods:
 *
 * - te.Type()	Return transaction element type (TR_ADDED|TR_REMOVED).
 * - te.N()	Return package name.
 * - te.E()	Return package epoch.
 * - te.V()	Return package version.
 * - te.R()	Return package release.
 * - te.A()	Return package architecture.
 * - te.O()	Return package operating system.
 * - te.NEVR()	Return package name-[epoch:]version-release.
 * - te.NEVRA()	Return package name-[epoch:]version-release.arch
 * - te.EVR()	Return package [epoch:]version-release.
 * - te.Color() Return package color bits.
 * - te.PkgFileSize() Return no. of bytes in package file (approx).
 * - te.Depth()	Return the level in the dependency tree (after ordering).
 * - te.Npreds() Return the number of package prerequisites (after ordering).
 * - te.Degree() Return the parent's degree + 1.
 * - te.Parent() Return the parent element index.
 * - te.Tree()	Return the root dependency tree index.
 * - te.AddedKey() Return the added package index (TR_ADDED).
 * - te.DependsOnKey() Return the package index for the added package (TR_REMOVED).
 * - te.DBOffset() Return the Packages database instance number (TR_REMOVED)
 * - te.Key()	Return the associated opaque key, i.e. 2nd arg ts.addInstall().
 * - te.DS(tag)	Return package dependency set.
 * @param tag	'Providename', 'Requirename', 'Obsoletename', 'Conflictname'
 * - te.FI(tag)	Return package file info set.
 * @param tag	'Basenames'
 */

static PyObject *
rpmte_TEType(rpmteObject * s)
{
    return Py_BuildValue("i", rpmteType(s->te));
}

static PyObject *
rpmte_N(rpmteObject * s)
{
    return PyString_FromString(rpmteN(s->te));
}

static PyObject *
rpmte_E(rpmteObject * s)
{
    return PyString_FromString(rpmteE(s->te));
}

static PyObject *
rpmte_V(rpmteObject * s)
{
    return PyString_FromString(rpmteV(s->te));
}

static PyObject *
rpmte_R(rpmteObject * s)
{
    return PyString_FromString(rpmteR(s->te));
}

static PyObject *
rpmte_A(rpmteObject * s)
{
    return PyString_FromString(rpmteA(s->te));
}

static PyObject *
rpmte_O(rpmteObject * s)
{
    return PyString_FromString(rpmteO(s->te));
}

static PyObject *
rpmte_NEVR(rpmteObject * s)
{
    return PyString_FromString(rpmteNEVR(s->te));
}

static PyObject *
rpmte_NEVRA(rpmteObject * s)
{
    return PyString_FromString(rpmteNEVRA(s->te));
}

static PyObject *
rpmte_EVR(rpmteObject * s)
{
    return PyString_FromString(rpmteEVR(s->te));
}

static PyObject *
rpmte_Color(rpmteObject * s)
{
    return Py_BuildValue("i", rpmteColor(s->te));
}

static PyObject *
rpmte_PkgFileSize(rpmteObject * s)
{
    return Py_BuildValue("K", rpmtePkgFileSize(s->te));
}

static PyObject *
rpmte_AddedKey(rpmteObject * s)
{
    return Py_BuildValue("i", rpmteAddedKey(s->te));
}

static PyObject *
rpmte_DependsOnKey(rpmteObject * s)
{
    return Py_BuildValue("i", rpmteDependsOnKey(s->te));
}

static PyObject *
rpmte_DBOffset(rpmteObject * s)
{
    return Py_BuildValue("i", rpmteDBOffset(s->te));
}

static PyObject *
rpmte_Key(rpmteObject * s)
{
    PyObject * Key;

    /* XXX how to insure this is a PyObject??? */
    Key = (PyObject *) rpmteKey(s->te);
    if (Key == NULL)
      Key = Py_None;
	Py_INCREF(Key);
    return Key;
}

static PyObject *
rpmte_DS(rpmteObject * s, PyObject * args, PyObject * kwds)
{
    PyObject * TagN = NULL;
    rpmds ds;
    rpmTag tag;
    char * kwlist[] = {"tag", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O:DS", kwlist, &TagN))
	return NULL;

    tag = tagNumFromPyObject(TagN);
    if (tag == RPMTAG_NOT_FOUND) {
	return NULL;
    }

    ds = rpmteDS(s->te, tag);
    if (ds == NULL) {
	Py_RETURN_NONE;
    }
    return (PyObject *) rpmds_Wrap(rpmdsLink(ds, "rpmte_DS"));
}

static PyObject *
rpmte_FI(rpmteObject * s, PyObject * args, PyObject * kwds)
{
    rpmfi fi = rpmteFI(s->te);
    if (fi == NULL) {
	Py_RETURN_NONE;
    }
    return rpmfi_Wrap(rpmfiLink(fi, "rpmte_FI"));
}

/** \ingroup py_c
 */
static struct PyMethodDef rpmte_methods[] = {
    {"Type",	(PyCFunction)rpmte_TEType,	METH_NOARGS,
"te.Type() -> Type\n\
- Return element type (rpm.TR_ADDED | rpm.TR_REMOVED).\n" },
    {"N",	(PyCFunction)rpmte_N,		METH_NOARGS,
"te.N() -> N\n\
- Return element name.\n" },
    {"E",	(PyCFunction)rpmte_E,		METH_NOARGS,
"te.E() -> E\n\
- Return element epoch.\n" },
    {"V",	(PyCFunction)rpmte_V,		METH_NOARGS,
"te.V() -> V\n\
- Return element version.\n" },
    {"R",	(PyCFunction)rpmte_R,		METH_NOARGS,
"te.R() -> R\n\
- Return element release.\n" },
    {"A",	(PyCFunction)rpmte_A,		METH_NOARGS,
"te.A() -> A\n\
- Return element arch.\n" },
    {"O",	(PyCFunction)rpmte_O,		METH_NOARGS,
"te.O() -> O\n\
- Return element os.\n" },
    {"NEVR",	(PyCFunction)rpmte_NEVR,	METH_NOARGS,
"te.NEVR() -> NEVR\n\
- Return element name-[epoch:]version-release.\n" },
    {"NEVRA",	(PyCFunction)rpmte_NEVRA,	METH_NOARGS,
"te.NEVRA() -> NEVRA\n\
- Return element name-[epoch:]version-release.arch.\n" },
    {"EVR",	(PyCFunction)rpmte_EVR,	METH_NOARGS,
"te.EVR() -> EVR\n\
- Return element [epoch:]version-release.\n" },
    {"Color",(PyCFunction)rpmte_Color,		METH_NOARGS,
	NULL},
    {"PkgFileSize",(PyCFunction)rpmte_PkgFileSize,	METH_NOARGS,
	NULL},
    {"AddedKey",(PyCFunction)rpmte_AddedKey,	METH_NOARGS,
	NULL},
    {"DependsOnKey",(PyCFunction)rpmte_DependsOnKey,	METH_NOARGS,
	NULL},
    {"DBOffset",(PyCFunction)rpmte_DBOffset,	METH_NOARGS,
	NULL},
    {"Key",	(PyCFunction)rpmte_Key,		METH_NOARGS,
	NULL},
    {"DS",	(PyCFunction)rpmte_DS,		METH_VARARGS|METH_KEYWORDS,
"te.DS(TagN) -> DS\n\
- Return the TagN dependency set (or None). TagN is one of\n\
	'Providename', 'Requirename', 'Obsoletename', 'Conflictname'\n" },
    {"FI",	(PyCFunction)rpmte_FI,		METH_VARARGS|METH_KEYWORDS,
"te.FI -> FI\n\
- Return element file info set'.\n" },
    {NULL,		NULL}		/* sentinel */
};

/* ---------- */

static int
rpmte_print(rpmteObject * s, FILE * fp, int flags)
{
    const char * tstr;
    if (!(s && s->te))
	return -1;
    switch (rpmteType(s->te)) {
    case TR_ADDED:	tstr = "++";	break;
    case TR_REMOVED:	tstr = "--";	break;
    default:		tstr = "??";	break;
    }
    fprintf(fp, "%s %s", tstr, rpmteNEVRA(s->te));
    return 0;
}

/**
 */
static char rpmte_doc[] =
"";

/** \ingroup py_c
 */
PyTypeObject rpmte_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.te",			/* tp_name */
	sizeof(rpmteObject),		/* tp_size */
	0,				/* tp_itemsize */
	(destructor)0,		 	/* tp_dealloc */
	(printfunc) rpmte_print,	/* tp_print */
	(getattrfunc)0,		 	/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	0,				/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	PyObject_GenericGetAttr,        /* tp_getattro */
	PyObject_GenericSetAttr,        /* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	rpmte_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	rpmte_methods,			/* tp_methods */
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

rpmteObject * rpmte_Wrap(rpmte te)
{
    rpmteObject *s = PyObject_New(rpmteObject, &rpmte_Type);
    if (s == NULL) {
	return PyErr_NoMemory();
    }
    s->te = te;
    return s;
}
