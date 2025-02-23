#include "apifn.h"
#include <stdio.h>

// Static values for testing
PyObject* py_get_online_players(PyObject* self, PyObject* args) {
    PyObject* player_list = PyList_New(2);
    PyList_SetItem(player_list, 0, PyUnicode_FromString("Bob"));
    PyList_SetItem(player_list, 1, PyUnicode_FromString("Sarah"));

    printf("Returning static online players: Bob, Sarah\n");  // Debug print
    return player_list;
}

PyObject* py_get_server_status(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("Running");  // Static value
}
