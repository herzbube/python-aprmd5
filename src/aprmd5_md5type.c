/*
 * Copyright 2009 Patrick Näf
 *
 * This file is part of python-aprmd5
 *
 * python-aprmd5 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * python-aprmd5 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with python-aprmd5. If not, see <http://www.gnu.org/licenses/>.
*/


// ---------------------------------------------------------------------------
// This file implements the md5 type exposed to Python.
// ---------------------------------------------------------------------------


// Project includes
#include "aprmd5.h"
#include "aprmd5_md5type.h"
#include "aprmd5_helpers.h"

// Defines PyMemberDef and PyMethodDef. Weird - why is this not in Python.h?
// See http://bugs.python.org/issue2897.
#include <structmember.h>


// ---------------------------------------------------------------------------
// Various strings that are exposed to Python and visible to the user
// ---------------------------------------------------------------------------

const char* aprmd5_md5_type_name = "md5";
static char* aprmd5_md5_init_kwlist[] = {"input", NULL};


// ---------------------------------------------------------------------------
// Definition of the C type that is used to create md5 objects
// ---------------------------------------------------------------------------

typedef struct {
  // Adds reference count and a pointer to the actual type object
  PyObject_HEAD
  // Type-specific fields go here
  apr_md5_ctx_t context;  // keeps state between calls to init(), update()
                          // and final()
} aprmd5_md5_object;


// ---------------------------------------------------------------------------
// Allocation/initialization/deallocation of md5 objects
// ---------------------------------------------------------------------------

// This function is responsible for creating objects *before* they are
// initialized by obj.__init__(). It is exposed in Python as
// class.__new__() method. __new__() is guaranteed to be called, even when the
// object is unpickled.
static PyObject*
aprmd5_md5_object_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
  aprmd5_md5_object* self = (aprmd5_md5_object*)type->tp_alloc(type, 0);
  if (NULL == self)
    return NULL;
  apr_status_t status = apr_md5_init(&self->context);
  if (status)
  {
    Py_DECREF(self);
    return NULL;
  }
  return (PyObject*)self;
}

// This function is responsible for initializing objects *after* they have been
// created by class.__new__(). It is exposed in Python as obj.__init__() method.
// __init__() is not guaranteed to be called: It is not called when an object
// is unpickled, and subclasses may also play dirty and not call __init__().
static int
aprmd5_md5_object_init(aprmd5_md5_object* self, PyObject* args, PyObject* kwds)
{
  // Get optional keyword argument
#if PY_MAJOR_VERSION >= 3
  // Input must be a bytes() object from which we can get a char*
  const char* format = "|y#";
#else
  // Input must be a str() object from which we can get a char*. The string may
  // contain null bytes.
  const char* format = "|s#";
#endif
  const char* input = NULL;
  Py_ssize_t inputLen = 0;
  if (! PyArg_ParseTupleAndKeywords(args, kwds, format, aprmd5_md5_init_kwlist, &input, &inputLen))
    return -1;
  // If there is input, feed it to the MD5 algorithm
  if (input != NULL)
  {
    apr_status_t status = apr_md5_update(&self->context, input, inputLen);
    if (status)
    {
      PyErr_SetString(PyExc_RuntimeError, "apr_md5_update() returned status code != 0");
      return -1;
    }
  }

  return 0;
}

// This function is responsible for freeing memory and resources when objects
// are destroyed.
static void
aprmd5_md5_object_dealloc(aprmd5_md5_object* self)
{
  // Free object memory; note that self might be a subclass instance (if we
  // allow subclassing)
#if PY_MAJOR_VERSION >= 3
  Py_TYPE(self)->tp_free((PyObject*)self);
#else   // #if PY_MAJOR_VERSION >= 3
  self->ob_type->tp_free((PyObject*)self);
#endif  // #if PY_MAJOR_VERSION >= 3
}


// ---------------------------------------------------------------------------
// Implementation of md5 type methods
// ---------------------------------------------------------------------------

static PyObject*
aprmd5_md5_object_update(aprmd5_md5_object* self, PyObject* args)
{
  // TODO: We do this sizeof() check because later on, when we call
  // apr_md5_update(), we are making an implicit type conversion from Py_ssize_t
  // (inputLen) to apr_size_t (function argument type). Instead of aborting
  // here, a better approach would probably be to iterate over the input buffer
  // and call apr_md5_update() multiple times
  assert(sizeof(apr_size_t) >= sizeof(Py_ssize_t));
  if (sizeof(apr_size_t) < sizeof(Py_ssize_t))
  {
    PyErr_SetString(PyExc_RuntimeError, "Function aborted, sizeof(apr_size_t) < sizeof(Py_ssize_t)");
    return NULL;
  }

#if PY_MAJOR_VERSION >= 3
  // Input must be a bytes() object from which we can get a char*
  const char* format = "y#";
#else
  // Input must be a str() object from which we can get a char*. The string may
  // contain null bytes.
  const char* format = "s#";
#endif
  const char* input;
  Py_ssize_t inputLen;
  if (! PyArg_ParseTuple(args, format, &input, &inputLen))
    return NULL;

  // Feed the input to the MD5 algorithm
  apr_status_t status = apr_md5_update(&self->context, input, inputLen);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_update() returned status code != 0");
    return NULL;
  }

  // Don't return NULL, as this would indicate an error
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
aprmd5_md5_object_digest(aprmd5_md5_object* self, PyObject* args)
{
  // Make a local copy of the context that we can operate on; apr_md5_final()
  // will zero that copy, but the original state in self remains untouched
  apr_md5_ctx_t contextCopy = self->context;

  // Generate the hash
  unsigned char digest[APRMD5_MD5_DIGESTSIZE];
  apr_status_t status = apr_md5_final(digest, &contextCopy);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_final() returned status code != 0");
    return NULL;
  }

#if PY_MAJOR_VERSION >= 3
  // Output must be a bytes() object
  const char* format = "y#";
#else
  // Output must be a str() object. The string may contain null bytes.
  const char* format = "s#";
#endif
  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue(format, digest, APRMD5_MD5_DIGESTSIZE);
}

static PyObject*
aprmd5_md5_object_hexdigest(aprmd5_md5_object* self, PyObject* args)
{
  // Make a local copy of the context that we can operate on; apr_md5_final()
  // will zero that copy, but the original state in self remains untouched so
  // that the user can continue calling update()
  apr_md5_ctx_t contextCopy = self->context;

  // Generate the hash
  unsigned char digest[APRMD5_MD5_DIGESTSIZE];
  apr_status_t status = apr_md5_final(digest, &contextCopy);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_final() returned status code != 0");
    return NULL;
  }

  // Construct the hex digest from the binary digest
  Py_ssize_t hexDigestLen = APRMD5_MD5_DIGESTSIZE * 2;
  char hexDigest[hexDigestLen];
  aprmd5_helper_bindigest_to_hexdigest(APRMD5_MD5_DIGESTSIZE, digest, hexDigest);

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s#", hexDigest, hexDigestLen);
}

static PyObject*
aprmd5_md5_object_copy(aprmd5_md5_object* self, PyObject* args)
{
  PyTypeObject* type = Py_TYPE(self);
  if (type != &aprmd5_md5_type)
    return NULL;
  aprmd5_md5_object* newobj = (aprmd5_md5_object*)type->tp_alloc(type, 0);
  if (NULL == newobj)
    return NULL;
  newobj->context = self->context;
  return (PyObject*)newobj;
}


// ---------------------------------------------------------------------------
// Implementation of md5 type attribute getters/setters
// ---------------------------------------------------------------------------

static PyObject *
aprmd5_md5_object_get_digest_size(aprmd5_md5_object* self, void* closure)
{
  return PyLong_FromLong(APRMD5_MD5_DIGESTSIZE);
}

static PyObject *
aprmd5_md5_object_get_block_size(aprmd5_md5_object* self, void* closure)
{
  return PyLong_FromLong(APRMD5_MD5_BLOCKSIZE);
}

static PyObject *
aprmd5_md5_object_get_name(aprmd5_md5_object* self, void* closure)
{
  return PyUnicode_FromStringAndSize(aprmd5_md5_type_name, strlen(aprmd5_md5_type_name));
}


// ---------------------------------------------------------------------------
// Attributes and methods of md5
// ---------------------------------------------------------------------------

static PyMemberDef aprmd5_md5_object_members[] =
{
  {NULL}  // Sentinel
};

static PyGetSetDef aprmd5_md5_object_getseters[] =
{
  {
    "digest_size",
    (getter)aprmd5_md5_object_get_digest_size, NULL,
    "The size of the resulting hash in bytes.",
    NULL
  },
  {
    "block_size",
    (getter)aprmd5_md5_object_get_block_size, NULL,
    "The internal block size of the hash algorithm in bytes.",
    NULL
  },
  {
    "name",
    (getter)aprmd5_md5_object_get_name, NULL,
    NULL,
    NULL
  },
  {NULL}  /* Sentinel */
};

static PyMethodDef aprmd5_md5_object_methods[] =
{
  {
    "update", (PyCFunction)aprmd5_md5_object_update, METH_VARARGS,
    "Update the hash object with the object arg, which must be interpretable as a buffer of bytes. Repeated calls are equivalent to a single call with the concatenation of all the arguments: m.update(a); m.update(b) is equivalent to m.update(a+b)."
  },
  {
    "digest", (PyCFunction)aprmd5_md5_object_digest, METH_NOARGS,
    "Return the digest of the data passed to the update() method so far. This is a bytes array of size digest_size which may contain bytes in the whole range from 0 to 255."
  },
  {
    "hexdigest", (PyCFunction)aprmd5_md5_object_hexdigest, METH_NOARGS,
    "Like digest() except the digest is returned as a string object of double length, containing only hexadecimal digits. This may be used to exchange the value safely in email or other non-binary environments."
  },
  {
    "copy", (PyCFunction)aprmd5_md5_object_copy, METH_NOARGS,
    "Return a copy (“clone”) of the hash object. This can be used to efficiently compute the digests of data sharing a common initial substring."
  },
  {NULL}  // Sentinel
};

// ---------------------------------------------------------------------------
// Definition of the Python type
// ---------------------------------------------------------------------------

#if PY_MAJOR_VERSION >= 3

PyTypeObject aprmd5_md5_type =
{
  PyObject_HEAD_INIT(NULL)
  "aprmd5.md5",                  // tp_name
  sizeof(aprmd5_md5_object),     // tp_basicsize
  0,                             // tp_itemsize
  (destructor)
    aprmd5_md5_object_dealloc,   // tp_dealloc
  0,                             // tp_print
  0,                             // tp_getattr
  0,                             // tp_setattr
  0,                             // tp_reserved
  0,                             // tp_repr
  0,                             // tp_as_number
  0,                             // tp_as_sequence
  0,                             // tp_as_mapping
  0,                             // tp_hash
  0,                             // tp_call
  0,                             // tp_str
  0,                             // tp_getattro
  0,                             // tp_setattro
  0,                             // tp_as_buffer
  Py_TPFLAGS_DEFAULT,            // tp_flags; Py_TPFLAGS_DEFAULT enables all
                                 // members defined by the version of Python
                                 // that this is compiled for
  "md5 objects",                 // tp_doc
  0,                             // tp_traverse
  0,                             // tp_clear
  0,                             // tp_richcompare
  0,                             // tp_weaklistoffset
  0,                             // tp_iter
  0,                             // tp_iternext
  aprmd5_md5_object_methods,     // tp_methods
  aprmd5_md5_object_members,     // tp_members
  aprmd5_md5_object_getseters,   // tp_getset
  0,                             // tp_base
  0,                             // tp_dict
  0,                             // tp_descr_get
  0,                             // tp_descr_set
  0,                             // tp_dictoffset
  (initproc)
    aprmd5_md5_object_init,      // tp_init
  0,                             // tp_alloc
  aprmd5_md5_object_new,         // tp_new
};

#else   // #if PY_MAJOR_VERSION >= 3

PyTypeObject aprmd5_md5_type =
{
  PyObject_HEAD_INIT(NULL)
  0,                             // ob_size
  "aprmd5.md5",                  // tp_name
  sizeof(aprmd5_md5_object),     // tp_basicsize
  0,                             // tp_itemsize
  (destructor)
    aprmd5_md5_object_dealloc,   // tp_dealloc
  0,                             // tp_print
  0,                             // tp_getattr
  0,                             // tp_setattr
  0,                             // tp_compare
  0,                             // tp_repr
  0,                             // tp_as_number
  0,                             // tp_as_sequence
  0,                             // tp_as_mapping
  0,                             // tp_hash
  0,                             // tp_call
  0,                             // tp_str
  0,                             // tp_getattro
  0,                             // tp_setattro
  0,                             // tp_as_buffer
  Py_TPFLAGS_DEFAULT,            // tp_flags; Py_TPFLAGS_DEFAULT enables all
                                 // members defined by the version of Python
                                 // that this is compiled for
  "md5 objects",                 // tp_doc
  0,                             // tp_traverse
  0,                             // tp_clear
  0,                             // tp_richcompare
  0,                             // tp_weaklistoffset
  0,                             // tp_iter
  0,                             // tp_iternext
  aprmd5_md5_object_methods,     // tp_methods
  aprmd5_md5_object_members,     // tp_members
  aprmd5_md5_object_getseters,   // tp_getset
  0,                             // tp_base
  0,                             // tp_dict
  0,                             // tp_descr_get
  0,                             // tp_descr_set
  0,                             // tp_dictoffset
  (initproc)
    aprmd5_md5_object_init,      // tp_init
  0,                             // tp_alloc
  aprmd5_md5_object_new,         // tp_new
};

#endif  // #if PY_MAJOR_VERSION >= 3
