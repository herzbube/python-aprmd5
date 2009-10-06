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


"""Unit tests for aprmd5.md5_encode()"""

# PSL
import unittest

# python-aprmd5
from aprmd5 import md5_encode


class MD5EncodeTest(unittest.TestCase):
    """Exercise aprmd5.md5_encode()"""

    def testNormal(self):
        password = "foo"
        salt = "mYJd83wW"
        expectedResult = "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        result = md5_encode(password, salt)
        self.assertEqual(result, expectedResult)

    def testPasswordIsEmptyString(self):
        password = ""
        salt = "7n4Iu7Bq"
        expectedResult = "$apr1$7n4Iu7Bq$jsH1cRc.tyRPvJpZjxUjV."
        result = md5_encode(password, salt)
        self.assertEqual(result, expectedResult)

    def testPasswordIsNone(self):
        password = None
        salt = "7n4Iu7Bq"
        self.assertRaises(TypeError, md5_encode, password, salt)

    def testSaltIsEmptyString(self):
        password = "foo"
        salt = ""
        expectedResult = "$apr1$$vGRl2mLvDG8pptkZ9Cyum."
        result = md5_encode(password, salt)
        self.assertEqual(result, expectedResult)

    def testSaltIsLongerThan8Characters(self):
        # Same password as in testNormal()
        password = "foo"
        # Same salt as in testNormal(), but with added characters
        salt = "mYJd83wW9876543210"
        # Same result as in testNormal
        expectedResult = "$apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50"
        result = md5_encode(password, salt)
        self.assertEqual(result, expectedResult)

    def testSaltIsNone(self):
        password = "foo"
        salt = None
        self.assertRaises(TypeError, md5_encode, password, salt)


if __name__ == "__main__":
    unittest.main()
