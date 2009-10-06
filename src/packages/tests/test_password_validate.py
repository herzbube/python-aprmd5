# encoding=utf-8

# Copyright 2009 Patrick Näf
# 
# This file is part of python-aprmd5
#
# python-aprmd5 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# python-aprmd5 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with python-aprmd5. If not, see <http://www.gnu.org/licenses/>.


"""Unit tests for aprmd5.password_validate()"""

# PSL
import unittest

# python-aprmd5
from aprmd5 import password_validate


class PasswordValidateTest(unittest.TestCase):
    """Exercise aprmd5.password_validate()"""

    def testValidationSucceeds(self):
        password = "foo"
        hash = "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = True
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testValidationFails(self):
        password = "foo"
        hash = "$apr1$mYJd83wW$xxxxxxxxxxxxxxxxxxxxxx"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testPasswordIsEmptyString(self):
        password = ""
        hash = "$apr1$7n4Iu7Bq$jsH1cRc.tyRPvJpZjxUjV."
        expectedResult = True
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testPasswordIsNone(self):
        password = None
        hash = "7n4Iu7Bq"
        self.assertRaises(TypeError, password_validate, password, hash)

    def testHashHasWrongPrefix(self):
        password = "foo"
        hash = "$apr2$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasEmptyPrefix(self):
        password = "foo"
        hash = "$$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasNoPrefix(self):
        password = "foo"
        hash = "mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasTooLongPrefix(self):
        password = "foo"
        hash = "$apr1234567890$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasEmptySalt(self):
        password = "foo"
        hash = "$apr1$$vGRl2mLvDG8pptkZ9Cyum."
        expectedResult = True
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasNoSalt(self):
        password = "foo"
        hash = "$apr1$vGRl2mLvDG8pptkZ9Cyum."
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasTooLongSalt(self):
        password = "foo"
        hash = "$apr1$mYJd83wW9876543210$IO.6aK3G0d4mHxcImhPX50"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasEmptyCrypt(self):
        password = "foo"
        hash = "$apr1$mYJd83wW$"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasNoCrypt(self):
        password = "foo"
        hash = "$apr1$mYJd83wW"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashHasTooLongCrypt(self):
        password = "foo"
        hash = "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX501234567890"
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashIsEmptyString(self):
        password = "foo"
        hash = ""
        expectedResult = False
        result = password_validate(password, hash)
        self.assertEqual(result, expectedResult)

    def testHashIsNone(self):
        password = "foo"
        hash = None
        self.assertRaises(TypeError, password_validate, password, hash)


if __name__ == "__main__":
    unittest.main()
