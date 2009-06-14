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

/*
 * This file wraps certain functions of the apr-util library and exposes them
 * as functions in the Python module "aprmd5".
 *
 * TODO: Document wrapped/exposed functions.
 */

// Make "s#" use Py_ssize_t rather than int
#define PY_SSIZE_T_CLEAN

// As per Python docs: This must be included before any standard headers are
// included.
#include <Python.h>

#include <apr-0/apr_md5.h>


// This is the wrapper for the function apr_md5_encode(), included from
// apr_md5.h. This wrapper will be available as aprmd5.md5_encode().
//
// Generates a salted crypt-style hash (crypt-apr1); for input "foo" and salt
// "mYJd83wW", the result is this: "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
static PyObject*
aprmd5_md5_encode(PyObject* self, PyObject* args)
{
  // Both the input and the salt must be str() objects, from which we can get
  // NULL-terminated char*.
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

// This is the wrapper for the function apr_md5(), included from apr_md5.h.
// This wrapper will be available as aprmd5.md5().
//
// Generates a real MD5 hash; for input "foo", the result is this:
// "acbd18db4cc2f85cedef654fccc4a4d8"
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

  // Input must be a bytes() object from which we can get a char*
  const char* input;
  Py_ssize_t inputLen;
  if (! PyArg_ParseTuple(args, "y#", &input, &inputLen))
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
  int i = 0;
  int j = 0;
  while (i < APR_MD5_DIGESTSIZE)
  {
    // Make sure that each byte results in a two digit hex number (padded by
    // leading 0, if necessary)
    sprintf(&(hexDigest[j]), "%02x", digest[i]);
    ++i;
    j += 2;
  }

  // Return the result; the Python system becomes responsible for the object
  // returned by Py_BuildValue().
  return Py_BuildValue("s#", hexDigest, hexDigestLen);
}


// The method table: List methods in this module.
static PyMethodDef AprMD5Methods[] =
{
  {
    "md5_encode", aprmd5_md5_encode, METH_VARARGS,
    "Encode a password using an MD5 algorithm."
  },
  {
    "md5", aprmd5_md5, METH_VARARGS,
    "Generate MD5 hash in one step."
  },
  {NULL, NULL, 0, NULL}   // Sentinel
};


// The module definition structure.
// Note: This exists only for Py3k
#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef aprmd5module =
{
  PyModuleDef_HEAD_INIT,
  "aprmd5",     // name of module
  NULL,         // module documentation, may be NULL
  -1,           // size of per-interpreter state of the module
                // or -1 if the module keeps state in global variables
  AprMD5Methods
};

#endif  // #if PY_MAJOR_VERSION >= 3


// The module’s initialization function. The initialization function must be
// named PyInit_name, where name is the name of the module, and should be the
// only non-static item defined in the module file. The function is called when
// the Python program imports the module for the first time.
PyMODINIT_FUNC

#if PY_MAJOR_VERSION >= 3

PyInit_aprmd5(void)
{
  return PyModule_Create(&aprmd5module);
}


#else   // #if PY_MAJOR_VERSION >= 3

initaprmd5(void)
{
  (void) Py_InitModule("aprmd5", AprMD5Methods);
}

#endif  // #if PY_MAJOR_VERSION >= 3
