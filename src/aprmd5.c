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
// This file contains the module definition.
// ---------------------------------------------------------------------------


// Project includes
#include "aprmd5.h"
#include "aprmd5_wrappers.h"
#include "aprmd5_md5type.h"


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
  aprmd5_md5_type.tp_new = PyType_GenericNew;
  // Initializes the type
  if (PyType_Ready(&aprmd5_md5_type) < 0)
    return NULL;

  PyObject* module = PyModule_Create(&aprmd5_module);
  if (NULL == module)
    return NULL;

  Py_INCREF(&aprmd5_md5_type);
  PyModule_AddObject(module, aprmd5_md5_type_name, (PyObject*)&aprmd5_md5_type);

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
