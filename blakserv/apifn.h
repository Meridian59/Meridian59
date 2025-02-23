#ifndef APIFN_H
#define APIFN_H

#include <Python.h>

// Function prototypes
PyObject* py_get_online_players(PyObject* self, PyObject* args);
PyObject* py_get_server_status(PyObject* self, PyObject* args);

#endif // APIFN_H
