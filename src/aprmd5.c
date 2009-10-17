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


/* ---------------------------------------------------------------------------
 * This file wraps those functions of the Apache Portable Runtime (APR) Utility
 * Library (libaprutil) that are known as the library's MD5 routines, and
 * exposes the wrapped functions in the Python module "aprmd5".
 *
 * MD5 routines that are omitted/not exposed: apr_md5_set_xlate().
 * ---------------------------------------------------------------------------
 */

// If this is defined, PyArg_ParseTuple() will use Py_ssize_t rather than int
// when it encounters the format string "s#".
// As per Python docs: "It is best to always define PY_SSIZE_T_CLEAN" because
// in some future version of Python, support for int will be dropped completely.
#define PY_SSIZE_T_CLEAN

// As per Python docs: This must be included before any standard headers are
// included.
#include <Python.h>

// Defines PyMemberDef and PyMethodDef. Weird - why is this not in Python.h?
// See http://bugs.python.org/issue2897.
#include <structmember.h>


// The filename of the header that we include depends on the version of
// libaprutil that we want to use, and the platform we are on. We want this C
// source file to remain independent, so we use a preprocessor macro and
// delegate responsibility for defining the macro to the build process. The
// usual place where the macro is defined is setup.py.
#include APR_MD5_HEADER_FILENAME

// TODO: This global variable is used to keep the state between function calls
// of apr_md5_init(), apr_md5_update() and apr_md5_final(). Very ugly, should be
// improved so that the aprmd5 module returns md5 objects that encapsulate this
// state.
static apr_md5_ctx_t aprmd5_module_context;

// The digest size is defined by libaprutil
#define APRMD5_MD5_DIGESTSIZE   APR_MD5_DIGESTSIZE
// The block size is *not* defined by libaprutil; assume the same value as in
// Python's hashlib (see <pythonroot>/Modules/md5module.c)
#define APRMD5_MD5_BLOCKSIZE    64


// ---------------------------------------------------------------------------
// Converts a binary MD5 digest into its corresponding hexadecimal MD5 digest.
//
// Parameters:
// - binDigest: The binary digest, is expected to be a character array of length
//   APR_MD5_DIGESTSIZE (i.e. the length is determined at compile time, by that
//   version of libaprutil that this file is compiled against)
// - hexDigest: A pre-allocated character array whose content is overwritten by
//   this function with the hexadecimal MD5 digest. The array is expected to be
//   of length APR_MD5_DIGESTSIZE * 2 (because each byte of the binary digest
//   will be converted into two hexadecimal digits/characters).
//
// Return value:
// - None
//
// Note: This is a helper function only, it is not exposed by the Python aprmd5
// module.
// ---------------------------------------------------------------------------
void aprmd5_helper_bindigest_to_hexdigest(const unsigned char* binDigest, char* hexDigest)
{
  int i = 0;
  int j = 0;
  while (i < APR_MD5_DIGESTSIZE)
  {
    // Make sure that each byte results in a two digit hex number (padded by
    // leading 0, if necessary)
    sprintf(&(hexDigest[j]), "%02x", binDigest[i]);
    ++i;
    j += 2;
  }
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_md5_encode(), included from
// apr_md5.h. From within Python, this wrapper will be available as
//
//   aprmd5.md5_encode()
//
// Generates a salted crypt-style hash. For input "foo" and salt "mYJd83wW",
// the result is this: "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50".
//
// Parameters of the Python function:
// - password: a string object that contains the password to be encrypted
// - salt: a string object that contains the salt to be used for encryption; the
//   htpasswd command line utility has been observed to use 8 character salts,
//   which at the same time is the maximum length that makes any difference (it
//   is possible to provide salts that are longer, but all characters beyond the
//   8th character will be ignored); the salt may be less than 8 characters
//   (even zero)
//
// Return value of the Python function:
// - A string object that contains the encrypted password in the format
//   "$apr1$salt$encrypted-password". Note that the prefix "$apr1$" is a
//   constant that distinguishes this form of crypt-style hash from other
//   crypt-style hashes, which use other prefixes (e.g. the original DES crypt
//   does not have a prefix at all, while "$1$" denotes yet another MD5-based
//   hash that is, for instance, implemented by the glibc function crypt(3)).
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_md5_encode(PyObject* self, PyObject* args)
{
  // Both the input and the salt must be str() objects, from which we can get
  // NULL-terminated char*.
  // Note: This is true for both Python 2.6 and Python 3
  const char* input;
  const char* salt;
  if (! PyArg_ParseTuple(args, "ss", &input, &salt))
    return NULL;

  // Generate the hash
  apr_size_t resultLen = 6 + strlen(salt) + 1 + 22 + 1;  // 6 = $apr1$
                                                         // 1 = $ (terminating the salt)
                                                         // 22 = hash
                                                         // 1 = terminating null byte
  char result[resultLen];
  // +1 to resultLen because, for some unknown reason, apr_md5_encode() wants
  // an additional byte
  apr_status_t status = apr_md5_encode(input, salt, result, resultLen + 1);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_encode() returned status code != 0");
    return NULL;
  }

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s", result);
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_password_validate(), included from
// apr_md5.h. From within Python, this wrapper will be available as
//
//   aprmd5.password_validate()
//
// Parameters of the Python function:
// - password: a string object that contains the password in clear text to be
//   validated
// - hash: a string object that contains the encrypted password against which
//   the password in clear text should be validated; the format must be
//   "$apr1$salt$encrypted-password", i.e. the same format as the output of the
//   md5_encode() function.
//
// Return value of the Python function:
// - True: If the validation was successful, i.e. if hash is indeed the
//   encrypted form of password
// - False: If the validation failed
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_password_validate(PyObject* self, PyObject* args)
{
  // Both the password and the hash must be str() objects, from which we can get
  // NULL-terminated char*.
  // Note: This is true for both Python 2.6 and Python 3
  const char* password;
  const char* hash;
  if (! PyArg_ParseTuple(args, "ss", &password, &hash))
    return NULL;

  // Validate the password against the given hash. A zero return status means
  // that the password is valid
  // Note: There are two pre-fabricated objects to represent the boolean values
  // True and False. Because of this, and because Py_BuildValue does not have a
  // special format string to refer to boolean values, we simply use the format
  // string "O" and pass in one of the pre-fabricated values. Py_BuildValue will
  // increase the refcount for us.
  apr_status_t status = apr_password_validate(password, hash);
  if (status)
    return Py_BuildValue("O", Py_False);
  else
    return Py_BuildValue("O", Py_True);
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_md5(), included from apr_md5.h.
// From within Python, this wrapper will be available as
//
//   aprmd5.md5()
//
// Generates a hash using the original, unmodified MD5 algorithm. For input
// "foo", the result is this: "acbd18db4cc2f85cedef654fccc4a4d8".
//
// Parameters of the Python function:
// - input: a string (Python 2.6 and earlier) or bytes (Python 3) object that
//   contains the message that a digest should be generated for
//
// Return value of the Python function:
// - The MD5 hash, in hexadecimal form, that was generated from input. Note:
//   The original libaprutil function apr_md5() returns a binary digest, not a
//   hexadecimal digest.
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_md5(PyObject* self, PyObject* args)
{
  // TODO: We do this sizeof() check because later on, when we call apr_md5(),
  // we are making an implicit type conversion from Py_ssize_t (inputLen) to
  // apr_size_t (function argument type). Instead of aborting here, a better
  // approach would probably be to iterate over the input buffer and use the
  // apr-util functions apr_md5_init(), apr_md5_update() and apr_md5_final().
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

  // Generate the hash
  unsigned char digest[APR_MD5_DIGESTSIZE];
  apr_status_t status = apr_md5(digest, input, inputLen);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5() returned status code != 0");
    return NULL;
  }

  // Construct the hex digest from the binary digest
  Py_ssize_t hexDigestLen = APR_MD5_DIGESTSIZE * 2;
  char hexDigest[hexDigestLen];
  aprmd5_helper_bindigest_to_hexdigest(digest, hexDigest);

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s#", hexDigest, hexDigestLen);
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_md5_init(), included from
// apr_md5.h. From within Python, this wrapper will be available as
//
//   aprmd5.md5_init()
//
// Initiates a new MD5 operation. This function is intended to be used in
// conjunction with md5_update() and md5_final(). When this function is called,
// any input that has been passed into md5_update() is discarded.
//
// Parameters of the Python function:
// - None
//
// Return value of the Python function:
// - None
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_md5_init(PyObject* self, PyObject* args)
{
  apr_status_t status = apr_md5_init(&aprmd5_module_context);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_init() returned status code != 0");
    return NULL;
  }

  // Don't return NULL, as this would indicate an error
  Py_INCREF(Py_None);
  return Py_None;
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_md5_update(), included from
// apr_md5.h. From within Python, this wrapper will be available as
//
//   aprmd5.md5_update()
//
// Adds input to an MD5 operation that has previously been initiated with
// md5_init(). This function can be called repeatedly to incrementally add more
// input to the MD5 operation. When all input has been added, md5_final() can
// be called to retrieve the MD5 digest.
//
// Parameters of the Python function:
// - input: a string (Python 2.6 and earlier) or bytes (Python 3) object that
//   contains part of the message that a digest should be generated for
//
// Return value of the Python function:
// - None
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_md5_update(PyObject* self, PyObject* args)
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

  // Feed the input into the MD5 algorithm
  apr_status_t status = apr_md5_update(&aprmd5_module_context, input, inputLen);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_update() returned status code != 0");
    return NULL;
  }

  // Don't return NULL, as this would indicate an error
  Py_INCREF(Py_None);
  return Py_None;
}


// ---------------------------------------------------------------------------
// This is the wrapper for the function apr_md5_final(), included from
// apr_md5.h. From within Python, this wrapper will be available as
//
//   aprmd5.md5_final()
//
// Concludes an MD5 operation that has previously been initiated with md5_init()
// and for which input has previously been added with md5_update(). This
// function needs to be called to retrieve the result, i.e. the digest, of the
// entire MD5 operation.
//
// Note: md5_init() must be called after this function to initiate a new
// MD5 operation. The result of calling md5_update() or md5_final() again,
// without first calling md5_init(), is undefined!
//
// Parameters of the Python function:
// - None
//
// Return value of the Python function:
// - The MD5 hash, in hexadecimal form, that was generated from all the input
//   fed into the MD5 operation by previous calls to md5_update(). Note: The
//   original libaprutil function apr_md5_final() returns a binary digest, not a
//   hexadecimal digest.
// ---------------------------------------------------------------------------
static PyObject*
aprmd5_md5_final(PyObject* self, PyObject* args)
{
  // Generate the hash
  unsigned char digest[APR_MD5_DIGESTSIZE];
  apr_status_t status = apr_md5_final(digest, &aprmd5_module_context);
  if (status)
  {
    PyErr_SetString(PyExc_RuntimeError, "apr_md5_init() returned status code != 0");
    return NULL;
  }

  // Construct the hex digest from the binary digest
  Py_ssize_t hexDigestLen = APR_MD5_DIGESTSIZE * 2;
  char hexDigest[hexDigestLen];
  aprmd5_helper_bindigest_to_hexdigest(digest, hexDigest);

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s#", hexDigest, hexDigestLen);
}

// ---------------------------------------------------------------------------
// Definition of the C type that represents MD5 objects
// ---------------------------------------------------------------------------

typedef struct {
	// Adds reference count and a pointer to the actual type object
  PyObject_HEAD
  // Type-specific fields go here
  apr_md5_ctx_t context;  // keeps state between calls to init(), update()
                          // and final()
} aprmd5_md5Object;


// ---------------------------------------------------------------------------
// Allocation/deallocation of MD5 objects
// ---------------------------------------------------------------------------

// This function is responsible for creating objects *before* they are
// initialized by obj.__init__(). It is exposed in Python as
// class.__new__() method. __new__() is guaranteed to be called, even if the
// object is unpickled.
static PyObject *
aprmd5_md5Object_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
  fprintf(stdout, "new called\n");
  aprmd5_md5Object* self = (aprmd5_md5Object*)type->tp_alloc(type, 0);
  if (self != NULL)
  {
    apr_status_t status = apr_md5_init(&self->context);
    if (status)
    {
      Py_DECREF(self);
      return NULL;
    }
  }
  return (PyObject*)self;
}

// This function is responsible for initializing objects *after* they have been
// created by class.__new__(). It is exposed in Python as obj.__init__() method.
// __init__() is not guaranteed to be called: it is not called when an object
// is unpickled, and subclasses may also play dirty and not call __init__().
static int
aprmd5_md5Object_init(aprmd5_md5Object* self, PyObject* args, PyObject* kwds)
{
  fprintf(stdout, "init called, context is %d, digest_size = %d, block_size = %d\n", self->context, APRMD5_MD5_DIGESTSIZE, APRMD5_MD5_BLOCKSIZE);

  // Make sure that no arguments have been passed
  static char* kwlist[] = {NULL};
  if (! PyArg_ParseTupleAndKeywords(args, kwds, "|", kwlist))
    return -1;
  if (0 == 0)   // TODO xxx replace this by some useful check
  {
    // We should never get here. If we do, aprmd5_md5Object_new() has not
    // been called!
    assert(false);
    apr_status_t status = apr_md5_init(&self->context);
    if (status)
    {
      PyErr_SetString(PyExc_RuntimeError, "apr_md5_init() returned status code != 0");
      return -1;
    }
  }
  return 0;
}

static void
aprmd5_md5Object_dealloc(aprmd5_md5Object* self)
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
// Implementation of methods of MD5 objects
// ---------------------------------------------------------------------------

// Forward declaration
static PyTypeObject aprmd5_md5Type;

// Helper
static aprmd5_md5Object *
new_md5Object(void)
{
  return (aprmd5_md5Object*)PyObject_New(aprmd5_md5Object, &aprmd5_md5Type);
}

static PyObject*
aprmd5_md5Object_update(aprmd5_md5Object* self, PyObject* args)
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

  // Feed the input into the MD5 algorithm
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
aprmd5_md5Object_digest(aprmd5_md5Object* self, PyObject* args)
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

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("y#", digest, APRMD5_MD5_DIGESTSIZE);
}

static PyObject*
aprmd5_md5Object_hexdigest(aprmd5_md5Object* self, PyObject* args)
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
  aprmd5_helper_bindigest_to_hexdigest(digest, hexDigest);

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s#", hexDigest, hexDigestLen);
}

static PyObject*
aprmd5_md5Object_copy(aprmd5_md5Object* self, PyObject* args)
{
  if (Py_TYPE(self) != &aprmd5_md5Type)
    return NULL;
  aprmd5_md5Object* newobj = new_md5Object();
  if (NULL == newobj)
    return NULL;
  newobj->context = self->context;
  return (PyObject *)newobj;

  return NULL;
}

static PyObject *
aprmd5_md5Object_get_digest_size(aprmd5_md5Object* self, void* closure)
{
  return PyLong_FromLong(APRMD5_MD5_DIGESTSIZE);
}

static PyObject *
aprmd5_md5Object_get_block_size(aprmd5_md5Object* self, void* closure)
{
  return PyLong_FromLong(APRMD5_MD5_BLOCKSIZE);
}

static PyObject *
aprmd5_md5Object_get_name(aprmd5_md5Object* self, void* closure)
{
  return PyUnicode_FromStringAndSize("md5", 3);
}


// ---------------------------------------------------------------------------
// Attributes and methods of MD5 objects
// ---------------------------------------------------------------------------

static PyMemberDef aprmd5_md5Object_members[] =
{
  {NULL}  // Sentinel
};

static PyGetSetDef aprmd5_md5Object_getseters[] =
{
  {
    "digest_size",
    (getter)aprmd5_md5Object_get_digest_size, NULL,
    "The size of the resulting hash in bytes.",
    NULL
  },
  {
    "block_size",
    (getter)aprmd5_md5Object_get_block_size, NULL,
    "The internal block size of the hash algorithm in bytes.",
    NULL
  },
  {
    "name",
    (getter)aprmd5_md5Object_get_name, NULL,
    NULL,
    NULL
  },
  {NULL}  /* Sentinel */
};

static PyMethodDef aprmd5_md5Object_methods[] =
{
  {
    "update", (PyCFunction)aprmd5_md5Object_update, METH_VARARGS,
    "Update the hash object with the object arg, which must be interpretable as a buffer of bytes. Repeated calls are equivalent to a single call with the concatenation of all the arguments: m.update(a); m.update(b) is equivalent to m.update(a+b)."
  },
  {
    "digest", (PyCFunction)aprmd5_md5Object_digest, METH_NOARGS,
    "Return the digest of the data passed to the update() method so far. This is a bytes array of size digest_size which may contain bytes in the whole range from 0 to 255."
  },
  {
    "hexdigest", (PyCFunction)aprmd5_md5Object_hexdigest, METH_NOARGS,
    "Like digest() except the digest is returned as a string object of double length, containing only hexadecimal digits. This may be used to exchange the value safely in email or other non-binary environments."
  },
  {
    "copy", (PyCFunction)aprmd5_md5Object_copy, METH_NOARGS,
    "Return a copy (“clone”) of the hash object. This can be used to efficiently compute the digests of data sharing a common initial substring."
  },
  {NULL}  // Sentinel
};

// ---------------------------------------------------------------------------
// Definition of the Python type that represents MD5 objects
// ---------------------------------------------------------------------------

#if PY_MAJOR_VERSION >= 3

static PyTypeObject aprmd5_md5Type =
{
  PyObject_HEAD_INIT(NULL)
  "aprmd5.md5",                  // tp_name
  sizeof(aprmd5_md5Object),      // tp_basicsize
  0,                             // tp_itemsize
  (destructor)
    aprmd5_md5Object_dealloc,    // tp_dealloc
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
  aprmd5_md5Object_methods,      // tp_methods
  aprmd5_md5Object_members,      // tp_members
  aprmd5_md5Object_getseters,    // tp_getset
  0,                             // tp_base
  0,                             // tp_dict
  0,                             // tp_descr_get
  0,                             // tp_descr_set
  0,                             // tp_dictoffset
  (initproc)
    aprmd5_md5Object_init,       // tp_init
  0,                             // tp_alloc
  aprmd5_md5Object_new,          // tp_new
};

#else   // #if PY_MAJOR_VERSION >= 3

static PyTypeObject aprmd5_md5Type =
{
  PyObject_HEAD_INIT(NULL)
  0,                             // ob_size
  "aprmd5.md5",                  // tp_name
  sizeof(aprmd5_md5Object),      // tp_basicsize
  0,                             // tp_itemsize
  (destructor)
    aprmd5_md5Object_dealloc,    // tp_dealloc
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
  aprmd5_md5Object_methods,      // tp_methods
  aprmd5_md5Object_members,      // tp_members
  aprmd5_md5Object_getseters,    // tp_getset
  0,                             // tp_base
  0,                             // tp_dict
  0,                             // tp_descr_get
  0,                             // tp_descr_set
  0,                             // tp_dictoffset
  (initproc)
    aprmd5_md5Object_init,       // tp_init
  0,                             // tp_alloc
  aprmd5_md5Object_new,          // tp_new
};

#endif  // #if PY_MAJOR_VERSION >= 3


// ---------------------------------------------------------------------------
// The method table: List methods in this module.
// ---------------------------------------------------------------------------
static PyMethodDef aprmd5_methods[] =
{
  {
    "md5_encode", aprmd5_md5_encode, METH_VARARGS,
    "Encode a password using an MD5 algorithm modified for Apache."
  },
  {
    "password_validate", aprmd5_password_validate, METH_VARARGS,
    "Validate any password encypted with any algorithm that APR understands."
  },
  {
    "md5", aprmd5_md5, METH_VARARGS,
    "Generate an MD5 hash in one step."
  },
  {
    "md5_init", aprmd5_md5_init, METH_VARARGS,
    "Begin an MD5 operation, writing a new context."
  },
  {
    "md5_update", aprmd5_md5_update, METH_VARARGS,
    "Continue an MD5 operation, processing another message block, and updating the context."
  },
  {
    "md5_final", aprmd5_md5_final, METH_VARARGS,
    "End an MD5 operation, generating the message digest and zeroing the context."
  },
  {NULL, NULL, 0, NULL}   // Sentinel
};


// ---------------------------------------------------------------------------
// The module definition structure.
// Note: This exists only for Py3k
// ---------------------------------------------------------------------------
#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef aprmd5_module =
{
  PyModuleDef_HEAD_INIT,
  "aprmd5",     // name of module
  NULL,         // module documentation, may be NULL
  -1,           // size of per-interpreter state of the module
                // or -1 if the module keeps state in global variables
  aprmd5_methods
};

#endif  // #if PY_MAJOR_VERSION >= 3


// ---------------------------------------------------------------------------
// The module’s initialization function. The initialization function must be
// named PyInit_name, where name is the name of the module, and should be the
// only non-static item defined in the module file. The function is called when
// the Python program imports the module for the first time.
// ---------------------------------------------------------------------------
PyMODINIT_FUNC

#if PY_MAJOR_VERSION >= 3

PyInit_aprmd5(void)
{
  // Allows new objects to be created
  aprmd5_md5Type.tp_new = PyType_GenericNew;
  // Initializes the type
  if (PyType_Ready(&aprmd5_md5Type) < 0)
    return NULL;

  PyObject* module = PyModule_Create(&aprmd5_module);
  if (NULL == module)
    return NULL;

  Py_INCREF(&aprmd5_md5Type);
  PyModule_AddObject(module, "md5", (PyObject*)&aprmd5_md5Type);

  return module;
}


#else   // #if PY_MAJOR_VERSION >= 3

initaprmd5(void)
{
  // Allows new objects to be created
  aprmd5_md5Type.tp_new = PyType_GenericNew;
  // Initializes the type
  if (PyType_Ready(&aprmd5_md5Type) < 0)
    return;

  PyObject* module = Py_InitModule("aprmd5", aprmd5_methods);
  if (NULL == module)
    return;

  Py_INCREF(&aprmd5_md5Type);
  PyModule_AddObject(module, "md5", (PyObject*)&aprmd5_md5Type);
}


#endif  // #if PY_MAJOR_VERSION >= 3
