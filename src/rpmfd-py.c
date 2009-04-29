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
