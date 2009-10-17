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
// This file defines common stuff; it's included by almost all the other
// files in the project.
// ---------------------------------------------------------------------------


#ifndef APRMD5_H
#define APRMD5_H

// If this is defined, PyArg_ParseTuple() will use Py_ssize_t rather than int
// when it encounters the format string "s#".
// As per Python docs: "It is best to always define PY_SSIZE_T_CLEAN" because
// in some future version of Python, support for int will be dropped completely.
#define PY_SSIZE_T_CLEAN

// As per Python docs: This must be included before any standard headers are
// included.
#include <Python.h>

// The filename of the header that we include depends on the version of
// libaprutil that we want to use, and the platform we are on. We want the C
// sources to remain independent, so we use a preprocessor macro and delegate
// responsibility for defining the macro to the build process. The usual place
// where the macro is defined is setup.py.
#include APRMD5_HEADER_FILENAME

// The digest size as defined by libaprutil
#define APRMD5_MD5_DIGESTSIZE   APR_MD5_DIGESTSIZE

// The block size is not defined by libaprutil; assume the same value as in
// Python's hashlib (see <pythonroot>/Modules/md5module.c)
#define APRMD5_MD5_BLOCKSIZE    64


#endif // #ifndef APRMD5_H
