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
// This file implements the functions that are direct wrappers around
// libaprutil's MD5 routines.
//
// Note: The following MD5 routines are omitted/not exposed:
// - apr_md5_set_xlate()
// - apr_md5()
// - apr_md5_init(), apr_md5_update() and apr_md5_final()
// ---------------------------------------------------------------------------


// Project includes
#include "aprmd5.h"
#include "aprmd5_wrappers.h"
#include "aprmd5_helpers.h"


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
PyObject*
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
  if (APR_SUCCESS != status)
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
PyObject*
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
  if (APR_SUCCESS != status)
    return Py_BuildValue("O", Py_False);
  else
    return Py_BuildValue("O", Py_True);
}


// ---------------------------------------------------------------------------
// The method table: List methods in this module.
// ---------------------------------------------------------------------------
PyMethodDef aprmd5_methods[] =
{
  {
    "md5_encode", aprmd5_md5_encode, METH_VARARGS,
    "Encode a password using an MD5 algorithm modified for the APR project."
  },
  {
    "password_validate", aprmd5_password_validate, METH_VARARGS,
    "Validate any password encrypted with any algorithm that APR understands."
  },
  {NULL, NULL, 0, NULL}   // Sentinel
};

