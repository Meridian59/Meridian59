#ifndef APIFN_H
#define APIFN_H

#include <Python.h>  // Include the Python headers

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
PyObject* py_get_online_players(PyObject* self, PyObject* args);
PyObject* py_get_server_status(PyObject* self, PyObject* args);
PyObject* py_signal_console(PyObject* self, PyObject* args);
PyObject* py_admin_page(PyObject* self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif // APIFN_H