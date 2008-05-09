/** \ingroup py_c
 * \file python/header-py.c
 */

#include <rpm/rpmlib.h>		/* rpmvercmp */
#include <rpm/rpmtag.h>
#include <rpm/rpmstring.h>
#include <rpm/rpmts.h>	/* XXX rpmtsCreate/rpmtsFree */

#include "header-py.h"
#include "rpmds-py.h"
#include "rpmfi-py.h"
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
    rpmTag tag;
    rpmTagType type;

    list = PyList_New(0);

    hi = headerInitIterator(s->h);
    while (headerNextIterator(hi, &tag, &type, NULL, NULL)) {
        if (tag == HEADER_I18NTABLE) continue;

	switch (type) {
	case RPM_BIN_TYPE:
	case RPM_INT32_TYPE:
	case RPM_CHAR_TYPE:
	case RPM_INT8_TYPE:
	case RPM_INT16_TYPE:
	case RPM_STRING_ARRAY_TYPE:
	case RPM_STRING_TYPE:
	    PyList_Append(list, o=PyInt_FromLong(tag));
	    Py_DECREF(o);
	    break;
	case RPM_I18NSTRING_TYPE: /* hum.. ?`*/
	case RPM_NULL_TYPE:
	default:
	    break;
	}
    }
    headerFreeIterator(hi);

    return list;
}

/** \ingroup py_c
 */
static PyObject * hdrUnload(hdrObject * s, PyObject * args, PyObject *keywords)
{
    char * buf;
    PyObject * rc;
    int len, legacy = 0;
    Header h;
    static char *kwlist[] = { "legacyHeader", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywords, "|i", kwlist, &legacy))
	return NULL;

    h = headerLink(s->h);
    /* XXX this legacy switch is a hack, needs to be removed. */
    if (legacy) {
	h = headerCopy(s->h);	/* XXX strip region tags, etc */
	headerFree(s->h);
    }
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
static PyObject * hdrSprintf(hdrObject * s, PyObject * args, PyObject * kwds)
{
    char * fmt;
    char * r;
    errmsg_t err;
    PyObject * result;
    char * kwlist[] = {"format", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &fmt))
	return NULL;

    r = headerSprintf(s->h, fmt, rpmTagTable, rpmHeaderFormats, &err);
    if (!r) {
	PyErr_SetString(pyrpmError, err);
	return NULL;
    }

    result = Py_BuildValue("s", r);
    free(r);

    return result;
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
    {"keys",		(PyCFunction) hdrKeyList,	METH_NOARGS,
	NULL },
    {"unload",		(PyCFunction) hdrUnload,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    {"sprintf",		(PyCFunction) hdrSprintf,	METH_VARARGS|METH_KEYWORDS,
	NULL },

    {"dsOfHeader",	(PyCFunction)hdr_dsOfHeader,	METH_NOARGS,
	NULL},
    {"dsFromHeader",	(PyCFunction)hdr_dsFromHeader,	METH_VARARGS|METH_KEYWORDS,
	NULL},
    {"fiFromHeader",	(PyCFunction)hdr_fiFromHeader,	METH_VARARGS|METH_KEYWORDS,
	NULL},

    {NULL,		NULL}		/* sentinel */
};

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
    char * str;

    if (PyInt_Check(item)) {
	return PyInt_AsLong(item);
    } else if (PyString_Check(item)) {
	str = PyString_AsString(item);
	return rpmTagGetValue(str);
    }
    return RPMTAG_NOT_FOUND;
}

/** \ingroup py_c
 * Retrieve tag info from header.
 * This is a "dressed" entry to headerGetEntry to do:
 *     1) DIRNAME/BASENAME/DIRINDICES -> FILENAMES tag conversions.
 *     2) i18n lookaside (if enabled).
 *
 * @param h            header
 * @param tag          tag
 * @retval type                address of tag value data type
 * @retval p           address of pointer to tag value(s)
 * @retval c           address of number of values
 * @return             0 on success, 1 on bad magic, 2 on error
 */
static int dressedHeaderGetEntry(Header h, rpmTag tag, rpmTagType *type,
	void **p, rpm_count_t *c)
{
    switch (tag) {
    case RPMTAG_GROUP:
    case RPMTAG_DESCRIPTION:
    case RPMTAG_SUMMARY:
    {	char *fmt;
	const char *msgstr, *errstr;

	fmt = rstrscat(NULL, "%{", rpmTagGetName(tag), "}\n", NULL);
	
	/* XXX FIXME: memory leak. */
        msgstr = headerSprintf(h, fmt, rpmTagTable, rpmHeaderFormats, &errstr);
	free(fmt);
	if (msgstr) {
	    *p = (void *) msgstr;
	    if (type)	*type = RPM_STRING_TYPE;
	    if (c)	*c = 1;
	    return 1;
	} else {
	    if (c)	*c = 0;
	    return 0;
	}
    }	break;

    default:
	return headerGetEntry(h, tag, type, p, c);
	break;
    }
}

/** \ingroup py_c
 */
static PyObject * hdr_subscript(hdrObject * s, PyObject * item)
{
    rpmTagType tagtype, type;
    rpmTag tag = RPMTAG_NOT_FOUND;
    rpm_count_t count, i;
    rpm_data_t data;
    PyObject * o, * metao;
    char ** stringArray;
    int forceArray = 0;
    int freeData = 0;
    char * str;
    const struct headerSprintfExtension_s * ext = NULL;
    const struct headerSprintfExtension_s * extensions = rpmHeaderFormats;

    if (PyCObject_Check (item))
        ext = PyCObject_AsVoidPtr(item);
    else
	tag = tagNumFromPyObject (item);
    if (tag == RPMTAG_NOT_FOUND && PyString_Check(item)) {
	/* if we still don't have the tag, go looking for the header
	   extensions */
	str = PyString_AsString(item);
	while (extensions->name) {
	    if (extensions->type == HEADER_EXT_TAG
	     && !rstrcasecmp(extensions->name + 7, str)) {
		ext = extensions;
	    }
	    extensions++;
	}
    }

    /* Retrieve data from extension or header. */
    if (ext) {
        ext->u.tagFunction(s->h, &type, &data, &count, &freeData);
    } else {
        if (tag == RPMTAG_NOT_FOUND) {
            PyErr_SetString(PyExc_KeyError, "unknown header tag");
            return NULL;
        }
        
	if (!dressedHeaderGetEntry(s->h, tag, &type, &data, &count)) {
	    switch (tag) {
	    case RPMTAG_EPOCH:
	    case RPMTAG_NAME:
	    case RPMTAG_VERSION:
	    case RPMTAG_RELEASE:
	    case RPMTAG_ARCH:
	    case RPMTAG_OS:
		Py_INCREF(Py_None);
		return Py_None;
		break;
	    default:
		return PyList_New(0);
		break;
	    }
	}
    }

    tagtype = rpmTagGetType(tag); 
#if NOTYET
    /* this blows up with header extension types */
    type = tagtype & RPM_MASK_TYPE;
#endif
    forceArray = (tagtype & RPM_MASK_RETURN_TYPE) == RPM_ARRAY_RETURN_TYPE;
    freeData = (tagtype & RPM_MASK_TYPE) == RPM_I18NSTRING_TYPE;

    switch (type) {
    case RPM_BIN_TYPE:
	o = PyString_FromStringAndSize(data, count);
	break;

    case RPM_INT32_TYPE:
	if (count != 1 || forceArray) {
	    metao = PyList_New(0);
	    for (i = 0; i < count; i++) {
		o = PyInt_FromLong(((int *) data)[i]);
		PyList_Append(metao, o);
		Py_DECREF(o);
	    }
	    o = metao;
	} else {
	    o = PyInt_FromLong(*((int *) data));
	}
	break;

    case RPM_CHAR_TYPE:
    case RPM_INT8_TYPE:
	if (count != 1 || forceArray) {
	    metao = PyList_New(0);
	    for (i = 0; i < count; i++) {
		o = PyInt_FromLong(((char *) data)[i]);
		PyList_Append(metao, o);
		Py_DECREF(o);
	    }
	    o = metao;
	} else {
	    o = PyInt_FromLong(*((char *) data));
	}
	break;

    case RPM_INT16_TYPE:
	if (count != 1 || forceArray) {
	    metao = PyList_New(0);
	    for (i = 0; i < count; i++) {
		o = PyInt_FromLong(((short *) data)[i]);
		PyList_Append(metao, o);
		Py_DECREF(o);
	    }
	    o = metao;
	} else {
	    o = PyInt_FromLong(*((short *) data));
	}
	break;

    case RPM_STRING_ARRAY_TYPE:
	stringArray = data;

	metao = PyList_New(0);
	for (i = 0; i < count; i++) {
	    o = PyString_FromString(stringArray[i]);
	    PyList_Append(metao, o);
	    Py_DECREF(o);
	}
	free (stringArray);
	o = metao;
	break;

    case RPM_STRING_TYPE:
    case RPM_I18NSTRING_TYPE:
	if (count != 1 || forceArray) {
	    stringArray = data;

	    metao = PyList_New(0);
	    for (i=0; i < count; i++) {
		o = PyString_FromString(stringArray[i]);
		PyList_Append(metao, o);
		Py_DECREF(o);
	    }
	    o = metao;
	} else {
	    o = PyString_FromString(data);
	    if (freeData)
		free (data);
	}
	break;

    default:
	PyErr_SetString(PyExc_TypeError, "unsupported type in header");
	return NULL;
    }

    return o;
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
    return PyObject_GenericSetAttr(o, n, v);
}

/** \ingroup py_c
 */
static PyMappingMethods hdr_as_mapping = {
	(lenfunc) 0,			/* mp_length */
	(binaryfunc) hdr_subscript,	/* mp_subscript */
	(objobjargproc)0,		/* mp_ass_subscript */
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
	0,				/* tp_new */
	0,				/* tp_free */
	0,				/* tp_is_gc */
};

hdrObject * hdr_Wrap(Header h)
{
    hdrObject * hdr = PyObject_New(hdrObject, &hdr_Type);
    hdr->h = headerLink(h);
    return hdr;
}

Header hdrGetHeader(hdrObject * s)
{
    return s->h;
}

/**
 */
PyObject * hdrLoad(PyObject * self, PyObject * args, PyObject * kwds)
{
    hdrObject * hdr;
    char * obj;
    Header h;
    int len;
    char * kwlist[] = {"headers", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &obj, &len))
	return NULL;

    /* copy is needed to avoid surprises from data swab in headerLoad(). */
    h = headerCopyLoad(obj);
    if (!h) {
	if (errno == ENOMEM) {
	    PyErr_SetString(PyExc_MemoryError, "out of memory");
	} else {
	    PyErr_SetString(pyrpmError, "bad header");
	}
	return NULL;
    }

    hdr = hdr_Wrap(h);
    h = headerFree(h);	/* XXX ref held by hdr */

    return (PyObject *) hdr;
}

/**
 */
PyObject * rpmReadHeaders (FD_t fd)
{
    PyObject * list;
    Header h;
    hdrObject * hdr;

    if (!fd) {
	PyErr_SetFromErrno(pyrpmError);
	return NULL;
    }

    list = PyList_New(0);
    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    while (h) {
	hdr = hdr_Wrap(h);
	if (PyList_Append(list, (PyObject *) hdr)) {
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
PyObject * rpmHeaderFromFD(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    int fileno;
    PyObject * list;
    char * kwlist[] = {"fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &fileno))
	return NULL;

    fd = fdDup(fileno);

    list = rpmReadHeaders (fd);
    Fclose(fd);

    return list;
}

/**
 */
PyObject * rpmHeaderFromFile(PyObject * self, PyObject * args, PyObject *kwds)
{
    char * filespec;
    FD_t fd;
    PyObject * list;
    char * kwlist[] = {"file", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &filespec))
	return NULL;

    fd = Fopen(filespec, "r.fdio");

    if (!fd) {
	PyErr_SetFromErrno(pyrpmError);
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
    int32_t * newMatch;
    int32_t * oldMatch;
    hdrObject * hdr;
    rpm_count_t c, count = 0;
    rpmTag tag;
    rpmTagType type;
    void * p;

    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    while (h) {
	if (!headerGetEntry(h, matchTag, NULL, (void **) &newMatch, NULL)) {
	    PyErr_SetString(pyrpmError, "match tag missing in new header");
	    return 1;
	}

	hdr = (hdrObject *) PyList_GetItem(list, count++);
	if (!hdr) return 1;

	if (!headerGetEntry(hdr->h, matchTag, NULL, (void **) &oldMatch, NULL)) {
	    PyErr_SetString(pyrpmError, "match tag missing in new header");
	    return 1;
	}

	if (*newMatch != *oldMatch) {
	    PyErr_SetString(pyrpmError, "match tag mismatch");
	    return 1;
	}

	for (hi = headerInitIterator(h);
	    headerNextIterator(hi, &tag, &type, (void *) &p, &c);
	    p = headerFreeData(p, type))
	{
	    /* could be dupes */
	    headerRemoveEntry(hdr->h, tag);
	    headerAddEntry(hdr->h, tag, type, p, c);
	}

	headerFreeIterator(hi);
	h = headerFree(h);

	Py_BEGIN_ALLOW_THREADS
	h = headerRead(fd, HEADER_MAGIC_YES);
	Py_END_ALLOW_THREADS
    }

    return 0;
}

PyObject *
rpmMergeHeadersFromFD(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    int fileno;
    PyObject * list;
    int rc;
    int matchTag;
    char * kwlist[] = {"list", "fd", "matchTag", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oii", kwlist, &list,
	    &fileno, &matchTag))
	return NULL;

    if (!PyList_Check(list)) {
	PyErr_SetString(PyExc_TypeError, "first parameter must be a list");
	return NULL;
    }

    fd = fdDup(fileno);

    rc = rpmMergeHeaders (list, fd, matchTag);
    Fclose(fd);

    if (rc) {
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/**
 */
PyObject *
rpmSingleHeaderFromFD(PyObject * self, PyObject * args, PyObject * kwds)
{
    FD_t fd;
    int fileno;
    off_t offset;
    PyObject * tuple;
    Header h;
    char * kwlist[] = {"fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &fileno))
	return NULL;

    offset = lseek(fileno, 0, SEEK_CUR);

    fd = fdDup(fileno);

    if (!fd) {
	PyErr_SetFromErrno(pyrpmError);
	return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    h = headerRead(fd, HEADER_MAGIC_YES);
    Py_END_ALLOW_THREADS

    Fclose(fd);

    tuple = PyTuple_New(2);

    if (h && tuple) {
	PyTuple_SET_ITEM(tuple, 0, (PyObject *) hdr_Wrap(h));
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

