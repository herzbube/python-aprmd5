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
// This file implements the module's helper functions.
// ---------------------------------------------------------------------------


// Project includes
#include "aprmd5_helpers.h"

// System includes
#include <stdio.h>  // for sprintf()


// ---------------------------------------------------------------------------
// Converts a binary MD5 digest into its corresponding hexadecimal MD5 digest.
//
// Parameters:
// - binDigestSize: The size of the binary digest in bytes
// - binDigest: The binary digest, is expected to be a character array of length
//   binDigestSize
// - hexDigest: A pre-allocated character array whose content is overwritten by
//   this function with the hexadecimal MD5 digest. The array is expected to be
//   of length binDigestSize * 2 (because each byte of the binary digest
//   will be converted into two hexadecimal digits/characters).
//
// Return value:
// - None
//
// Note: This is an internal helper function only, it is not exposed to Python.
// ---------------------------------------------------------------------------
void aprmd5_helper_bindigest_to_hexdigest(int binDigestSize, const unsigned char* binDigest, char* hexDigest)
{
  int i = 0;
  int j = 0;
  while (i < binDigestSize)
  {
    // Make sure that each byte results in a two digit hex number (padded by
    // leading 0, if necessary)
    sprintf(&(hexDigest[j]), "%02x", binDigest[i]);
    ++i;
    j += 2;
  }
}
