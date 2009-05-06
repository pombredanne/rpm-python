/** \ingroup py_c
 * \file python/header-py.c
 */

#include <rpm/rpmlib.h>		/* rpmvercmp */
#include <rpm/rpmtag.h>
#include <rpm/rpmstring.h>

#include "header-py.h"
#include "rpmds-py.h"
#include "rpmfi-py.h"
#include "rpmtd-py.h"
#include "rpmfd-py.h"
#include "rpmdebug-py.h"

/** \ingroup python
 * \class Rpm
 * \brief START HERE / RPM base module for the Python API
 *
 * The rpm base module provides the main starting point for
 * accessing RPM from Python. For most usage, call
 * the TransactionSet method to get a transaction set (rpmts).
 *
 * For example:
 * \code
 *	import rpm
 *
 *	ts = rpm.TransactionSet()
 * \endcode
 *
 * The transaction set will open the RPM database as needed, so
 * in most cases, you do not need to explicitly open the
 * database. The transaction set is the workhorse of RPM.
 *
 * You can open another RPM database, such as one that holds
 * all packages for a given Linux distribution, to provide
 * packages used to solve dependencies. To do this, use
 * the following code:
 *
 * \code
 * rpm.addMacro('_dbpath', '/path/to/alternate/database')
 * solvets = rpm.TransactionSet()
 * solvets.openDB()
 * rpm.delMacro('_dbpath')
 *
 * # Open default database
 * ts = rpm.TransactionSet()
 * \endcode
 *
 * This code gives you access to two RPM databases through
 * two transaction sets (rpmts): ts is a transaction set
 * associated with the default RPM database and solvets
 * is a transaction set tied to an alternate database, which
 * is very useful for resolving dependencies.
 *
 * The rpm methods used here are:
 *
 * - addMacro(macro, value)
 * @param macro   Name of macro to add
 * @param value   Value for the macro
 *
 * - delMacro(macro)
 * @param macro   Name of macro to delete
 *
 */

/** \ingroup python
 * \class Rpmhdr
 * \brief A python header object represents an RPM package header.
 *
 * All RPM packages have headers that provide metadata for the package.
 * Header objects can be returned by database queries or loaded from a
 * binary package on disk.
 *
 * The ts.hdrFromFdno() function returns the package header from a
 * package on disk, verifying package signatures and digests of the
 * package while reading.
 *
 * Note: The older method rpm.headerFromPackage() which has been replaced
 * by ts.hdrFromFdno() used to return a (hdr, isSource) tuple.
 *
 * If you need to distinguish source/binary headers, do:
 * \code
 * 	import os, rpm
 *
 *	ts = rpm.TranssactionSet()
 * 	fdno = os.open("/tmp/foo-1.0-1.i386.rpm", os.O_RDONLY)
 * 	hdr = ts.hdrFromFdno(fdno)
 *	os.close(fdno)
 *	if hdr[rpm.RPMTAG_SOURCEPACKAGE]:
 *	   print "header is from a source package"
 *	else:
 *	   print "header is from a binary package"
 * \endcode
 *
 * The Python interface to the header data is quite elegant.  It
 * presents the data in a dictionary form.  We'll take the header we
 * just loaded and access the data within it:
 * \code
 * 	print hdr[rpm.RPMTAG_NAME]
 * 	print hdr[rpm.RPMTAG_VERSION]
 * 	print hdr[rpm.RPMTAG_RELEASE]
 * \endcode
 * in the case of our "foo-1.0-1.i386.rpm" package, this code would
 * output:
\verbatim
  	foo
  	1.0
  	1
\endverbatim
 *
 * You make also access the header data by string name:
 * \code
 * 	print hdr['name']
 * 	print hdr['version']
 * 	print hdr['release']
 * \endcode
 *
 * This method of access is a teensy bit slower because the name must be
 * translated into the tag number dynamically. You also must make sure
 * the strings in header lookups don't get translated, or the lookups
 * will fail.
 */

/** \ingroup python
 * \name Class: rpm.hdr
 */

/** \ingroup py_c
 */
struct hdrObject_s {
    PyObject_HEAD
    Header h;
} ;

/** \ingroup py_c
 */
static PyObject * hdrKeyList(hdrObject * s)
{
    PyObject * list, *o;
    HeaderIterator hi;
    rpmtd td = rpmtdNew();

    list = PyList_New(0);

    hi = headerInitIterator(s->h);
    while (headerNext(hi, td)) {
	rpmTag tag = rpmtdTag(td);
	if (tag == HEADER_I18NTABLE) continue;

	switch (rpmtdType(td)) {
	case RPM_BIN_TYPE:
	case RPM_CHAR_TYPE:
	case RPM_INT8_TYPE:
	case RPM_INT16_TYPE:
	case RPM_INT32_TYPE:
	case RPM_INT64_TYPE:
	case RPM_STRING_ARRAY_TYPE:
	case RPM_STRING_TYPE:
	case RPM_I18NSTRING_TYPE: 
	    o = PyString_FromString(rpmTagGetName(tag));
	    PyList_Append(list, o);
	    Py_DECREF(o);
	    break;
	case RPM_NULL_TYPE:
	default:
	    break;
	}
    }
    headerFreeIterator(hi);
    rpmtdFree(td);

    return list;
}

static PyObject * hdrHasKey(hdrObject *self, PyObject *pytag)
{
    rpmTag tag = tagNumFromPyObject(pytag);
    if (tag == RPMTAG_NOT_FOUND) {
	return NULL;
    }

    return PyBool_FromLong(headerIsEntry(self->h, tag));
}

/** \ingroup py_c
 */
static PyObject * hdrUnload(hdrObject * s)
{
    char * buf;
    PyObject * rc;
    int len;
    Header h;

    h = headerLink(s->h);
    len = headerSizeof(h, 0);
    buf = headerUnload(h);
    h = headerFree(h);

    if (buf == NULL || len == 0) {
	PyErr_SetString(pyrpmError, "can't unload bad header\n");
	return NULL;
    }

    rc = PyString_FromStringAndSize(buf, len);
    free(buf);

    return rc;
}

/** \ingroup py_c
 */
static PyObject * hdrFormat(hdrObject * s, PyObject * args, PyObject * kwds)
{
    char * fmt;
    char * r;
    errmsg_t err;
    PyObject * result;
    char * kwlist[] = {"format", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &fmt))
	return NULL;

    r = headerFormat(s->h, fmt, &err);
    if (!r) {
	PyErr_SetString(PyExc_ValueError, err);
	return NULL;
    }

    result = PyString_FromString(r);
    free(r);

    return result;
}

/*
 * XXX TODO: 
 * - muchos overlap with hdr_subsrcipt(), unify... 
 * - flags unused for now, should support extension enable/disable and raw
 */
PyObject * hdrGet(hdrObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *pytag;
    char *kwlist[] = {"tag", "flags", NULL};
    char *flags;
    rpmtd td = NULL;
    rpmTag tag; 

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &pytag, &flags))
	return NULL;

    tag = tagNumFromPyObject(pytag);
    if (tag == RPMTAG_NOT_FOUND) {
	return NULL;
    }

    td = rpmtdNew();
    if (headerGet(self->h, tag, td, HEADERGET_EXT)) {
	return rpmtd_Wrap(td);
    }
    rpmtdFree(td);
    Py_RETURN_NONE;
}

PyObject *hdrPut(hdrObject *self, PyObject *args, PyObject *kwds)
{
    int rc;
    char *kwlist[] = {"td", "flags", NULL};
    headerPutFlags flags;
    rpmtdObject *tdo;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|i", kwlist, 
				     &rpmtd_Type, &tdo, &flags)) {
	return NULL;
    }
    rc = headerPut(self->h, tdo->td, HEADERPUT_DEFAULT);
    return PyBool_FromLong(rc);
}

PyObject *hdrConvert(hdrObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[] = {"op", NULL};
    headerConvOps op;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &op)) {
        return NULL;
    }

    return PyBool_FromLong(headerConvert(self->h, op));
}

static PyObject * hdr_write(hdrObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *fo = NULL;
    char *kwlist[] = { "file", "magic", NULL };
    int magic = 1;
    FD_t fd = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &fo, &magic)) {
	return NULL;
    }
    if ((fd = rpmFdFromPyObject(fo)) == NULL) {
	return NULL;
    }

    if (headerWrite(fd, hdrGetHeader(self), 
		    magic ? HEADER_MAGIC_YES : HEADER_MAGIC_NO)) {
	return PyErr_SetFromErrno(PyExc_IOError);
    }
    
    Py_RETURN_NONE;
}

/**
 */
static int hdr_compare(hdrObject * a, hdrObject * b)
{
    return rpmVersionCompare(a->h, b->h);
}

static long hdr_hash(PyObject * h)
{
    return (long) h;
}

/** \ingroup py_c
 */
static struct PyMethodDef hdr_methods[] = {
    {"get",		(PyCFunction) hdrGet,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {"put",		(PyCFunction) hdrPut,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {"has_key",		(PyCFunction) hdrHasKey,	METH_O,
	NULL },
    {"keys",		(PyCFunction) hdrKeyList,	METH_NOARGS,
	NULL },
    {"unload",		(PyCFunction) hdrUnload,	METH_NOARGS,
	NULL },
    {"format",		(PyCFunction) hdrFormat,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {"convert",		(PyCFunction) hdrConvert,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {"write",		(PyCFunction)hdr_write,		METH_VARARGS|METH_KEYWORDS,
	NULL },

    {NULL,		NULL}		/* sentinel */
};

static PyObject *hdr_new(PyTypeObject *subtype,
			 PyObject *args, PyObject *kwds)
{
    PyObject *obj = NULL;
    Header h = NULL;
    char *kwlist[] = { "obj", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist, &obj)) {
	return NULL;
    }

    if (obj == NULL) {
	h = headerNew();
    } else if (hdrObject_Check(obj)) {
	h = headerCopy(hdrGetHeader((hdrObject*) obj));
    } else if (PyString_Check(obj)) {
	h = headerCopyLoad(PyString_AsString(obj));
    } else if (PyFile_Check(obj)) {
	FD_t fd = rpmFdFromPyObject(obj);
	if (fd == NULL) {
	    return PyErr_SetFromErrno(PyExc_IOError);
	}
	Py_BEGIN_ALLOW_THREADS;
	h = headerRead(fd, HEADER_MAGIC_YES);
	Py_END_ALLOW_THREADS;
	Fclose(fd);
    } else {
	PyErr_SetNone(PyExc_TypeError);
	return NULL;
    }

    if (h == NULL) {
	PyErr_SetString(pyrpmError, "bad header");
	return NULL;
    }
    
    return hdr_Wrap(h);
}

/** \ingroup py_c
 */
static void hdr_dealloc(hdrObject * s)
{
    if (s->h) headerFree(s->h);
    PyObject_Del(s);
}

/** \ingroup py_c
 */
rpmTag tagNumFromPyObject (PyObject *item)
{
    rpmTag tag = RPMTAG_NOT_FOUND;

    if (PyInt_Check(item)) {
	/* XXX we should probably validate tag numbers too */
	tag = PyInt_AsLong(item);
    } else if (PyString_Check(item)) {
	tag = rpmTagGetValue(PyString_AsString(item));
    }
    if (tag == RPMTAG_NOT_FOUND) {
	PyErr_SetString(PyExc_ValueError, "unknown header tag");
    }
	
    return tag;
}


static PyObject * hdr_subscript(hdrObject *self, PyObject *item)
{
    PyObject *res = NULL;
    rpmtd td;
    rpmTag tag = tagNumFromPyObject(item);

    if (tag == RPMTAG_NOT_FOUND) {
	return NULL;
    }

    td = rpmtdNew();
    (void) headerGet(self->h, tag, td, HEADERGET_EXT);
    /* this knows how to handle empty containers and all */
    res = rpmtd_AsPyobj(td);

    rpmtdFreeData(td);
    rpmtdFree(td);
    return res;
}

static int hdrAppend(Header h, rpmTag tag, PyObject *value)
{
    rpmTagType type = rpmTagGetType(tag) & RPM_MASK_TYPE;
    int rc = 0;

    switch (type) {
    case RPM_STRING_TYPE:
    case RPM_I18NSTRING_TYPE:
    case RPM_STRING_ARRAY_TYPE:
	if (PyString_Check(value)) {
	    char *str = PyString_AsString(value);
	    rc = headerPutString(h, tag, str);
	}
	break;
    case RPM_INT64_TYPE: 
	if (PyLong_Check(value)) {
	    uint64_t num = PyLong_AsUnsignedLongLong(value);
	    rc = headerPutUint64(h, tag, &num, 1);
	}
	break;
    case RPM_INT32_TYPE: 
	if (PyInt_Check(value)) {
	    uint32_t num = PyInt_AsLong(value);
	    rc = headerPutUint32(h, tag, &num, 1);
	}
	break;
    case RPM_INT16_TYPE: 
	if (PyInt_Check(value)) {
	    uint16_t num = PyInt_AsLong(value);
	    rc = headerPutUint16(h, tag, &num, 1);
	}
	break;
    case RPM_INT8_TYPE: 
    case RPM_CHAR_TYPE: 
	if (PyInt_Check(value)) {
	    uint8_t num = PyInt_AsLong(value);
	    rc = headerPutUint8(h, tag, &num, 1);
	}
	break;
    case RPM_BIN_TYPE:
	if (PyString_Check(value)) {
	    uint8_t *blob = (uint8_t*)PyString_AsString(value);
	    rc = headerPutBin(h, tag, blob, PyString_Size(value));
	}
	break;
    default:
	PyErr_SetString(PyExc_KeyError, "unhandled data type");
	break;
    }
    return rc;
}

static int hdr_ass_subscript(hdrObject *self, PyObject *key, PyObject *value)
{
    rpmTag tag = tagNumFromPyObject(key);
    if (tag == RPMTAG_NOT_FOUND) {
	return -1;
    }

    if (value == NULL) {
	/* XXX raising keyerror here is inconsistent with other methods, wdo? */
	if (headerDel(self->h, tag)) {
	    PyErr_SetString(PyExc_KeyError, "no such tag in header");
	    return -1;
	}
    /* XXX TODO: need to be much more careful about accepted types.. */
    } else if (PyList_Check(value)) {
	Py_ssize_t i, len = PyList_Size(value);
	for (i = 0; i < len; i++) {
	    PyObject *item = PyList_GetItem(value, i);
	    if (hdrAppend(self->h, tag, item) != 1) {
		PyErr_SetString(PyExc_TypeError, "invalid data for tag");
		return -1;
	    }
	}
    } else {
	if (hdrAppend(self->h, tag, value) != 1) {
	    PyErr_SetString(PyExc_TypeError, "invalid data for tag");
	    return -1;
	}
    }
    return 0;
}

static PyObject * hdr_getattro(PyObject * o, PyObject * n)
{
    PyObject * res;
    res = PyObject_GenericGetAttr(o, n);
    if (res == NULL)
	res = hdr_subscript((hdrObject *)o, n);
    return res;
}

static int hdr_setattro(PyObject * o, PyObject * n, PyObject * v)
{
    int res = PyObject_GenericSetAttr(o, n, v);
    if (res != 0) 
	res = hdr_ass_subscript((hdrObject *)o, n, v);
    return res;
}

/** \ingroup py_c
 */
static PyMappingMethods hdr_as_mapping = {
	(lenfunc) 0,			/* mp_length */
	(binaryfunc) hdr_subscript,	/* mp_subscript */
	(objobjargproc) hdr_ass_subscript,	/* mp_ass_subscript */
};

/**
 */
static char hdr_doc[] =
"";

/** \ingroup py_c
 */
PyTypeObject hdr_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.hdr",			/* tp_name */
	sizeof(hdrObject),		/* tp_size */
	0,				/* tp_itemsize */
	(destructor) hdr_dealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	(getattrfunc) 0, 		/* tp_getattr */
	0,				/* tp_setattr */
	(cmpfunc) hdr_compare,		/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,	 			/* tp_as_sequence */
	&hdr_as_mapping,		/* tp_as_mapping */
	hdr_hash,			/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	(getattrofunc) hdr_getattro,	/* tp_getattro */
	(setattrofunc) hdr_setattro,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	hdr_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	hdr_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	0,				/* tp_init */
	0,				/* tp_alloc */
	hdr_new,			/* tp_new */
	0,				/* tp_free */
	0,				/* tp_is_gc */
};

PyObject * hdr_Wrap(Header h)
{
    hdrObject * hdr = PyObject_New(hdrObject, &hdr_Type);
    if (hdr == NULL) {
	return PyErr_NoMemory();
    }
    hdr->h = headerLink(h);
    return (PyObject*) hdr;
}

Header hdrGetHeader(hdrObject * s)
{
    return s->h;
}

/**
 */
PyObject * rpmReadHeaders (FD_t fd)
{
    PyObject *list, *hdr;
    Header h;

    if (!fd) {
	PyErr_SetFromErrno(PyExc_IOError);
	return NULL;
    }

    list = PyList_New(0);
    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    while (h) {
	hdr = hdr_Wrap(h);
	if (PyList_Append(list, hdr)) {
	    Py_DECREF(list);
	    Py_DECREF(hdr);
	    return NULL;
	}
	Py_DECREF(hdr);

	h = headerFree(h);	/* XXX ref held by hdr */

	Py_BEGIN_ALLOW_THREADS
	h = headerRead(fd, HEADER_MAGIC_YES);
	Py_END_ALLOW_THREADS
    }

    return list;
}

/**
 */
PyObject * rpmHeaderFromIO(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    PyObject * fo;
    PyObject * list;
    char * kwlist[] = {"fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &fo))
	return NULL;

    if ((fd = rpmFdFromPyObject(fo)) == NULL) {
	return NULL;
    }

    list = rpmReadHeaders (fd);
    Fclose(fd);

    return list;
}

/**
 * This assumes the order of list matches the order of the new headers, and
 * throws an exception if that isn't true.
 */
int rpmMergeHeaders(PyObject * list, FD_t fd, int matchTag)
{
    Header h;
    HeaderIterator hi;
    rpmTag newMatch, oldMatch;
    hdrObject * hdr;
    rpm_count_t count = 0;
    int rc = 1; /* assume failure */
    rpmtd td = rpmtdNew();

    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    while (h) {
	if (!headerGet(h, matchTag, td, HEADERGET_MINMEM)) {
	    PyErr_SetString(pyrpmError, "match tag missing in new header");
	    goto exit;
	}
	newMatch = rpmtdTag(td);
	rpmtdFreeData(td);

	hdr = (hdrObject *) PyList_GetItem(list, count++);
	if (!hdr) goto exit;

	if (!headerGet(hdr->h, matchTag, td, HEADERGET_MINMEM)) {
	    PyErr_SetString(pyrpmError, "match tag missing in new header");
	    goto exit;
	}
	oldMatch = rpmtdTag(td);
	rpmtdFreeData(td);

	if (newMatch != oldMatch) {
	    PyErr_SetString(pyrpmError, "match tag mismatch");
	    goto exit;
	}

	for (hi = headerInitIterator(h); headerNext(hi, td); rpmtdFreeData(td))
	{
	    /* could be dupes */
	    headerDel(hdr->h, rpmtdTag(td));
	    headerPut(hdr->h, td, HEADERPUT_DEFAULT);
	}

	headerFreeIterator(hi);
	h = headerFree(h);

	Py_BEGIN_ALLOW_THREADS
	h = headerRead(fd, HEADER_MAGIC_YES);
	Py_END_ALLOW_THREADS
    }
    rc = 0;

exit:
    td = rpmtdFree(td);

    return rc;
}

PyObject *
rpmMergeHeadersFromFD(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    PyObject * fo;
    PyObject * list;
    int rc;
    int matchTag;
    char * kwlist[] = {"list", "fd", "matchTag", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOi", kwlist, &list,
	    &fo, &matchTag))
	return NULL;

    if (!PyList_Check(list)) {
	PyErr_SetString(PyExc_TypeError, "first parameter must be a list");
	return NULL;
    }
    if ((fd = rpmFdFromPyObject(fo)) == NULL) {
	return NULL;
    }

    rc = rpmMergeHeaders (list, fd, matchTag);
    Fclose(fd);

    if (rc) {
	return NULL;
    }

    Py_RETURN_NONE;
}

/**
 */
PyObject *
rpmSingleHeaderFromFD(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    PyObject *fo = NULL;
    off_t offset;
    PyObject * tuple;
    Header h;
    char * kwlist[] = {"fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &fo))
	return NULL;

    if ((fd = rpmFdFromPyObject(fo)) == NULL) {
	return NULL;
    }

    offset = Fseek(fd, 0, SEEK_CUR);

    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    Fclose(fd);

    tuple = PyTuple_New(2);

    if (h && tuple) {
	PyTuple_SET_ITEM(tuple, 0, hdr_Wrap(h));
	PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(offset));
	h = headerFree(h);
    } else {
	Py_INCREF(Py_None);
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(tuple, 0, Py_None);
	PyTuple_SET_ITEM(tuple, 1, Py_None);
    }

    return tuple;
}

/**
 */
PyObject * versionCompare (PyObject * self, PyObject * args, PyObject * kwds)
{
    hdrObject * h1, * h2;
    char * kwlist[] = {"version0", "version1", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!O!", kwlist, &hdr_Type,
	    &h1, &hdr_Type, &h2))
	return NULL;

    return Py_BuildValue("i", hdr_compare(h1, h2));
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

PyObject * labelCompare (PyObject * self, PyObject * args)
{
    char *v1, *r1, *v2, *r2;
    const char *e1, *e2;
    int rc;

    if (!PyArg_ParseTuple(args, "(zzz)(zzz)",
			&e1, &v1, &r1, &e2, &v2, &r2))
	return NULL;

    if (e1 == NULL)	e1 = "0";
    if (e2 == NULL)	e2 = "0";

    rc = compare_values(e1, e2);
    if (!rc) {
	rc = compare_values(v1, v2);
	if (!rc)
	    rc = compare_values(r1, r2);
    }
    return Py_BuildValue("i", rc);
}

