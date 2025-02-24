#include <Python.h>  // Include the Python headers first
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN  // Prevent windows.h from including winsock.h
#include <windows.h>  // For Windows types
#include "blakserv.h" // For server types
#include "interface.h" // For interface functions
#include "apifn.h"    // For our function declarations

// Function to get online players
PyObject* py_get_online_players(PyObject* self, PyObject* args) {
    PyObject* player_list = PyList_New(2);
    PyList_SetItem(player_list, 0, PyUnicode_FromString("Bob"));
    PyList_SetItem(player_list, 1, PyUnicode_FromString("Sarah"));

    printf("Returning static online players: Bob, Sarah\n");  // Debug print
    return player_list;
}

// Function to get server status
PyObject* py_get_server_status(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("Running");  // Static value
}

// Function to signal the console
PyObject* py_signal_console(PyObject* self, PyObject* args) {
    // Call the InterfaceSignalConsole function
    InterfaceSignalConsole();

    // Return a success message
    return PyUnicode_FromString("InterfaceSignalConsole called successfully");
}