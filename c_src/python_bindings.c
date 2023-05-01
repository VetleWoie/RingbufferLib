#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <python3.10/numpy/arrayobject.h>

static PyObject *ringbuffer_init(PyObject *self, PyObject *args);
static PyObject *ringbuffer_read(PyObject *self, PyObject *args);
static PyObject *ringbuffer_write(PyObject *self, PyObject *args);
static PyObject *ringbuffer_read_copy(PyObject *self, PyObject *args);
static PyObject *ringbuffer_copy_stdin(PyObject *self, PyObject *args);

static PyMethodDef RingbufferMethods[] = {
    {"init",  ringbuffer_init, METH_VARARGS,
     "Initialize ringbuffer"},
    {"write",  ringbuffer_write, METH_VARARGS,
     "Write bytes to ringbuffer"},
    {"read",  ringbuffer_read, METH_VARARGS,
     "Read ringbuffer into numpy array"},
    {"read_copy",  ringbuffer_read_copy, METH_VARARGS,
     "Read ringbuffer by copying bytes into existing numpy array."},
    {"copy_stdin",  ringbuffer_copy_stdin, METH_VARARGS,
     "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef ringbuffermodule = {
    PyModuleDef_HEAD_INIT,
    "Ringbuffer",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    RingbufferMethods
};

PyMODINIT_FUNC PyInit_ringbuffer(void)
{
    import_array();
    return PyModule_Create(&ringbuffermodule);
}

