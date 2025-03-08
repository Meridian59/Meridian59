#include "api_dispatch.h"
#include "apifn.h"  // Import the real function declarations

#include <stdio.h>

PyObject* api_get_online_players(PyObject* self, PyObject* args) {
    return py_get_online_players(self, args);  // Calls the real function in apifn.c
}

PyObject* api_get_server_status(PyObject* self, PyObject* args) {
    return py_get_server_status(self, args);  // Calls the real function in apifn.c
}

PyObject* api_signal_console(PyObject* self, PyObject* args) {
    return py_signal_console(self, args);  // Calls the real function in apifn.c
}

// Define method table
static PyMethodDef ApiMethods[] = {
    {"get_online_players", api_get_online_players, METH_NOARGS, "Get online players"},
    {"get_server_status", api_get_server_status, METH_NOARGS, "Get server status"},
    {"signal_console", api_signal_console, METH_NOARGS, "Signal the console"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// Define module
static struct PyModuleDef apimodule = {
    PyModuleDef_HEAD_INIT,
    "api_dispatch",
    NULL,
    -1,
    ApiMethods
};

// Module initialization
PyMODINIT_FUNC PyInit_api_dispatch(void) {
    return PyModule_Create(&apimodule);
}