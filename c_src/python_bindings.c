#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <python3.10/numpy/arrayobject.h>
#include "ringbuffer.h"

typedef struct {
    PyObject_HEAD
    ringbuffer_t *ringbuffer;
} RingbufferObject;

static PyObject *py_ringbuffer_read(RingbufferObject *self, PyObject *args);
static PyObject *py_ringbuffer_write(RingbufferObject *self, PyObject *args);
static PyObject *py_ringbuffer_read_copy(RingbufferObject *self, PyObject *args);
static PyObject *ringbuffer_copy_stdin(RingbufferObject *self, PyObject *args);

static int ringbuffer_init(RingbufferObject *self, PyObject *args);
static void ringbuffer_dealloc(RingbufferObject *self);


static PyMethodDef Ringbuffer_methods[] = {
    {"write", (PyCFunction) py_ringbuffer_write, METH_VARARGS,
     "Write bytes to ringbuffer"
    },
    {"read", (PyCFunction) py_ringbuffer_read, METH_VARARGS,
     "Read bytes from ringbuffer into new numpy array of specified type"
    },
    {"read_copy", (PyCFunction) py_ringbuffer_read_copy, METH_VARARGS,
     "Read bytes from ringbuffer into specified numpy array"
    },
    {"copy_stdin", (PyCFunction) ringbuffer_copy_stdin, METH_VARARGS,
     "Read bytes from stdin and write to ringbuffer"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject RingbufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ringbuffer.Ringbuffer",
    .tp_doc = PyDoc_STR("Ringbuffer"),
    .tp_basicsize = sizeof(RingbufferObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) ringbuffer_dealloc,
    .tp_init = (initproc) ringbuffer_init,
    .tp_methods = Ringbuffer_methods,
};


static PyMethodDef RingbufferMethods[] = {
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

PyMODINIT_FUNC PyInit_ringbuffer(void){
    PyObject *m;
    if (PyType_Ready(&RingbufferType) < 0)
        return NULL;

    m = PyModule_Create(&ringbuffermodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&RingbufferType);
    if (PyModule_AddObject(m, "Ringbuffer", (PyObject *) &RingbufferType) < 0) {
        Py_DECREF(&RingbufferType);
        Py_DECREF(m);
        return NULL;
    }
    import_array();

    return m;
}

/*
Deallocator for the new python object
*/
static void ringbuffer_dealloc(RingbufferObject *self){
    printf("Ringbuffer dealloc\n");
    if(self->ringbuffer != NULL){
        destroy_ringbuffer(self->ringbuffer);
    }
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/*
Init new ringbuffer object by allocating place for a new ringbuffer
*/
static int ringbuffer_init(RingbufferObject *self, PyObject *args){
    int max_size;
    char *path;
    char *mode;

    printf("Ringbuffer init\n");
    // If ringbuffer is allready allocated don't do anything
    if(self->ringbuffer != NULL){
        return 0;
    }

    if (!PyArg_ParseTuple(args,"izs", &max_size,&path,&mode)){
        return -1;
    }
    printf("max_size: %d ,path: %s, mode: %c\n",max_size, path, *mode);
    self->ringbuffer = init_ringbuffer(max_size, path, mode);
    return 0; 
}

/*
Read bytes from ringbuffer into new numpy array of specified type
*/
static PyObject *py_ringbuffer_read(RingbufferObject *self, PyObject *args){
    int num;
    PyArray_Descr* type;

    if (!PyArg_ParseTuple(args,"iO&", &num, PyArray_DescrConverter, &type)){
        return NULL;
    }
    
    int size_read = num * type->elsize;
    printf("num: %d, size: %d, size_read: %d\n", num, type->elsize, size_read);
    char *buf = malloc(size_read);
    ringbuffer_read(self->ringbuffer, buf, size_read);

    npy_intp dims[1] = {num};
    PyObject *arr = PyArray_SimpleNewFromData(1,dims,type->type_num, buf);
    
    return arr; 
}

static PyObject *py_ringbuffer_write(RingbufferObject *self, PyObject *args){
    const Py_buffer bytes;

    //Read bytes like object into Py_buffer 
    if(!PyArg_ParseTuple(args,"y*",&bytes)){
        return NULL;
    }
    ringbuffer_write(self->ringbuffer, bytes.buf, bytes.len);
    Py_RETURN_NONE; 
}

/*
Read bytes from ringbuffer into provided numpy array
*/
static PyObject *py_ringbuffer_read_copy(RingbufferObject *self, PyObject *args){
    PyArrayObject *arr;
    PyArray_Descr *descr;
    if (!PyArg_ParseTuple(args, "O!",&PyArray_Type, &arr))
        return NULL;
    
    if(PyArray_NDIM(arr) > 1){
        PyErr_SetString(PyExc_ValueError,"Array must be one dimensional");
        return NULL;
    }

    descr = PyArray_DESCR(arr);

    npy_intp num = PyArray_DIM(arr, 0);
    int size_read = num * descr->elsize;

    size_read = PyArray_NBYTES(arr);
    char *buf = (char *) PyArray_DATA(arr);
    ringbuffer_read(self->ringbuffer, buf, size_read);

    Py_RETURN_NONE; 
}
static PyObject *ringbuffer_copy_stdin(RingbufferObject *self, PyObject *args){
    char err_msg[50];
    char *buf;
    u_int32_t max_read;

    if (!PyArg_ParseTuple(args, "i",&max_read))
        return NULL;

    //Reopen STDIN as binary stream
    if(freopen(NULL, "rb", stdin) == NULL){
        sprintf(err_msg, "Could not reopen stdin");

        goto err;
    };

    buf = (char *) malloc(max_read);
    if(buf == NULL){
        return NULL;
    }
    //Start reading stdin
    while(1){
        ssize_t bytes_read = read(STDIN_FILENO,buf,max_read);
        if(bytes_read < 0){
            sprintf(err_msg, "Could not read stdin");
            goto err;
        }else if(buf[bytes_read-1] == EOF){
            printf("Got EOF shutting down\n");
            break;
        }
        ringbuffer_write(self->ringbuffer, buf, bytes_read);
    }

    Py_RETURN_NONE; 
    err:
        PyErr_SetString(PyExc_EnvironmentError,err_msg);
        return NULL;
}