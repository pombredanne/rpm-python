
#include <rpm/rpmstring.h>

#include "rpmfd-py.h"

FD_t rpmFdFromPyObject(PyObject *obj)
{
    FD_t fd = NULL;

    if (PyInt_Check(obj)) {
	fd = fdDup(PyInt_AsLong(obj));
    } else if (PyFile_Check(obj)) {
	FILE *fp = PyFile_AsFile(obj);
	fd = fdDup(fileno(fp));
    } else if (PyString_Check(obj)) {
	fd = Fopen(PyString_AsString(obj), "r.fdio");
    } else {
	PyErr_SetString(PyExc_TypeError, "integer or file object expected");
	return NULL;
    }
    if (fd == NULL || Ferror(fd)) {
	PyErr_SetFromErrno(PyExc_IOError);
    }
    return fd;
}

static PyObject *rpmfd_new(PyTypeObject *subtype, 
			   PyObject *args, PyObject *kwds)
{
    char *kwlist[] = { "obj", "mode", "flags", NULL };
    char *mode = "r";
    char *flags = "ufdio";
    PyObject *fo = NULL;
    rpmfdObject *self = NULL;
    FD_t fd = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ss", kwlist, 
				     &fo, &mode, &flags)) {
	return NULL;
    }

    if (PyString_Check(fo)) {
	char *m = rstrscat(NULL, mode, ".", flags, NULL);
	fd = Fopen(PyString_AsString(fo), m);
	free(m);
    } else if (PyInt_Check(fo)) {
	fd = fdDup(PyInt_AsLong(fo));
    } else {
	PyErr_SetString(PyExc_TypeError, "path or file descriptor expected");
	return NULL;
    }

    if (Ferror(fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(fd));
	return NULL;
    }

    if ((self = PyObject_New(rpmfdObject, subtype)) == NULL) {
	return PyErr_NoMemory();
    }
    self->fd = fd;
    return (PyObject*) self;
}

static void rpmfd_dealloc(rpmfdObject *self)
{
    if (self->fd) {
	Fclose(self->fd);
	self->fd = NULL;
	PyObject_Del(self);
    }
}

static PyObject *rpmfd_close(rpmfdObject *self)
{
    if (self->fd) {
	Fclose(self->fd);
	self->fd = NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *rpmfd_fileno(rpmfdObject *self)
{
    int fno = Fileno(self->fd);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	return NULL;
    }
    return Py_BuildValue("i", fno);
}

static PyObject *rpmfd_flush(rpmfdObject *self)
{
    int rc = Fflush(self->fd);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	return NULL;
    }
    return Py_BuildValue("i", rc);
}

static PyObject *rpmfd_isatty(rpmfdObject *self)
{
    int fileno = Fileno(self->fd);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	return NULL;
    }
    return PyBool_FromLong(isatty(fileno));
}

static PyObject *rpmfd_seek(rpmfdObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[] = { "offset", "whence", NULL };
    off_t offset;
    int whence = SEEK_SET;
    int rc = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|i", kwlist, 
				     &whence, &offset)) {
	return NULL;
    }

    rc = Fseek(self->fd, offset, whence);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	return NULL;
    }
    return Py_BuildValue("i", rc);
}

static PyObject *rpmfd_read(rpmfdObject *self, PyObject *args, PyObject *kwds)
{
    char *kwlist[] = { "size", NULL };
    char *buf = NULL;
    ssize_t reqsize = -1;
    ssize_t bufsize = 0;
    PyObject *res = NULL;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|l", kwlist, &reqsize)) {
	return NULL;
    }

    /* XXX simple, stupid for now ... and broken for anything large */
    bufsize = (reqsize < 0) ? fdSize(self->fd) : reqsize;
    if ((buf = malloc(bufsize+1)) == NULL) {
	return PyErr_NoMemory();
    }
    
    Fread(buf, 1, bufsize, self->fd);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	goto exit;
    }
    res = PyString_FromStringAndSize(buf, bufsize);

exit:
    free(buf);
    return res;
}

static PyObject *rpmfd_write(rpmfdObject *self, PyObject *args, PyObject *kwds)
{

    const char *buf = NULL;
    ssize_t size = 0;
    char *kwlist[] = { "buffer", NULL };
    ssize_t rc = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &buf, &size)) {
	return NULL;
    }

    rc = Fwrite(buf, 1, size, self->fd);
    if (Ferror(self->fd)) {
	PyErr_SetString(PyExc_IOError, Fstrerror(self->fd));
	return NULL;
    }
    return Py_BuildValue("n", rc);
}

static char rpmfd_doc[] = "";

static struct PyMethodDef rpmfd_methods[] = {
    { "close",	(PyCFunction) rpmfd_close,	METH_NOARGS,
	NULL },
    { "fileno",	(PyCFunction) rpmfd_fileno,	METH_NOARGS,
	NULL },
    { "fflush",	(PyCFunction) rpmfd_flush,	METH_NOARGS,
	NULL },
    { "isatty",	(PyCFunction) rpmfd_isatty,	METH_NOARGS,
	NULL },
    { "read",	(PyCFunction) rpmfd_read,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    { "seek",	(PyCFunction) rpmfd_seek,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    { "write",	(PyCFunction) rpmfd_write,	METH_VARARGS|METH_KEYWORDS,
	NULL },
    { NULL, NULL }
};

PyTypeObject rpmfd_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/* ob_size */
	"rpm.fd",			/* tp_name */
	sizeof(rpmfdObject),		/* tp_size */
	0,				/* tp_itemsize */
	/* methods */
	(destructor) rpmfd_dealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	(getattrfunc)0, 		/* tp_getattr */
	(setattrfunc)0,			/* tp_setattr */
	(cmpfunc)0,			/* tp_compare */
	(reprfunc)0,			/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	(hashfunc)0,			/* tp_hash */
	(ternaryfunc)0,			/* tp_call */
	(reprfunc)0,			/* tp_str */
	(getattrofunc)0,	/* tp_getattro */
	(setattrofunc)0,	/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	rpmfd_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	rpmfd_methods,			/* tp_methods */
	0,				/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc)0,			/* tp_init */
	(allocfunc)0,			/* tp_alloc */
	(newfunc) rpmfd_new,		/* tp_new */
	(freefunc)0,		/* tp_free */
	0,				/* tp_is_gc */
};
