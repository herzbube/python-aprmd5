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
// This file declares the md5 type exposed to Python.
// ---------------------------------------------------------------------------


#ifndef APRMD5_MD5TYPE_H
#define APRMD5_MD5TYPE_H


// Type name that is exposed to Python
extern const char* aprmd5_md5_type_name;

// Type object
extern PyTypeObject aprmd5_md5_type;


#endif // #ifndef APRMD5_MD5TYPE_H

