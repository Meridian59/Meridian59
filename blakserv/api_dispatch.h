#ifndef API_DISPATCH_H
#define API_DISPATCH_H

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes
PyObject* api_get_online_players(PyObject* self, PyObject* args);
PyObject* api_get_server_status(PyObject* self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif // API_DISPATCH_H
