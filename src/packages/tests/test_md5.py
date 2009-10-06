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


"""Unit tests for aprmd5.md5()"""

# PSL
import unittest

# python-aprmd5
from aprmd5 import md5
import tests   # import stuff from __init__.py (e.g. tests.python2)


class MD5Test(unittest.TestCase):
    """Exercise aprmd5.md5()"""

    def testNormal(self):
        if tests.python2:
            input = "foo"
        else:
            # Convert into bytes. We can use UTF-8 because we know that this
            # file, and therefore the literal "foo", is UTF-8 encoded.
            input = "foo".encode("utf-8")
        expectedResult = "acbd18db4cc2f85cedef654fccc4a4d8"
        result = md5(input)
        self.assertEqual(result, expectedResult)

    def testInputIsEmpty(self):
        if tests.python2:
            input = ""
        else:
            input = bytes()
        expectedResult = "d41d8cd98f00b204e9800998ecf8427e"
        result = md5(input)
        self.assertEqual(result, expectedResult)

    def testInputIsNone(self):
        input = None
        self.assertRaises(TypeError, md5, input)


if __name__ == "__main__":
    unittest.main()
