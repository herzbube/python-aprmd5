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


"""Unit tests for aprmd5.md5_init(), aprmd5.md5_update() and aprmd5.md5_final()"""

# PSL
import unittest

# python-aprmd5
from aprmd5 import md5_init, md5_update, md5_final
import tests   # import stuff from __init__.py (e.g. tests.python2)


class MD5OperationTest(unittest.TestCase):
    """Exercise aprmd5.md5_init(), aprmd5.md5_update() and aprmd5.md5_final()"""

    def setUp(self):
        if tests.python2:
            self.inputNormal = "foo"
            self.inputEmpty = ""
        else:
            # Convert into bytes. We can use UTF-8 because we know that this
            # file, and therefore the literal "foo", is UTF-8 encoded.
            self.inputNormal = "foo".encode("utf-8")
            self.inputEmpty = bytes()
        self.expectedResultNormal = "acbd18db4cc2f85cedef654fccc4a4d8"
        self.expectedResultInputEmpty = "d41d8cd98f00b204e9800998ecf8427e"
        self.expectedResultInputTwice = "fdba98970961edb29f88241b9d99d890"

    def testNormal(self):
        md5_init()
        md5_update(self.inputNormal)
        result = md5_final()
        self.assertEqual(result, self.expectedResultNormal)

    def testInputIsEmpty(self):
        md5_init()
        md5_update(self.inputEmpty)
        result = md5_final()
        self.assertEqual(result, self.expectedResultInputEmpty)

    def testInputIsNone(self):
        input = None
        md5_init()
        self.assertRaises(TypeError, md5_update, input)

    def testInitTwice(self):
        # Invoking md5_init() twice in a row is perfectly legal
        md5_init()
        md5_init()
        md5_update(self.inputNormal)
        result = md5_final()
        self.assertEqual(result, self.expectedResultNormal)

    def testUpdateTwice(self):
        # Invoking md5_update() two or more times is not only legal, it is a
        # normal use case
        md5_init()
        md5_update(self.inputNormal)
        md5_update(self.inputNormal)
        result = md5_final()
        self.assertEqual(result, self.expectedResultInputTwice)

    def testFinalTwice(self):
        # As far as I know, the result of invoking md5_final() a second time,
        # without having invoked md5_init() before, is not defined. At the
        # moment, the only thing we assert in this test is that the second
        # invocation of md5_final() does not spit out an error but gives us
        # *some* result; we don't make any assertion whatsoever about the actual
        # *value* of the result.
        # Note: It is very likely that the implementation is going to change at
        # some time in the future and spit out an error instead of just silently
        # producing an undefined result. If this happens, this test will need
        # to be changed as well.
        md5_init()
        md5_update(self.inputNormal)
        result1 = md5_final()
        self.assertEqual(result1, self.expectedResultNormal)
        result2 = md5_final()
        # The second invocation of md5_final is ***NOT*** a non-operation!
        self.assertNotEqual(result2, self.expectedResultNormal)

    def testUpdateAfterFinal(self):
        # Again, the result of not invoking md5_init() after md5_final() is
        # undefined. See notes of the above test for details.
        md5_init()
        md5_update(self.inputNormal)
        result1 = md5_final()
        self.assertEqual(result1, self.expectedResultNormal)
        md5_update(self.inputNormal)
        result2 = md5_final()
        # It's ***NOT*** the same as updating with normal input twice in a row!
        self.assertNotEqual(result2, self.expectedResultInputTwice)

    def testNoUpdate(self):
        # Not invoking md5_update() is the same as invoking it with empty input
        md5_init()
        result = md5_final()
        self.assertEqual(result, self.expectedResultInputEmpty)

    def testInitAfterUpdate(self):
        # Invoking md5_init() after md5_update(), but without invoking
        # md5_final(), is perfectly legal and simply resets the whole context
        md5_init()
        md5_update(self.inputNormal)
        md5_init()
        result = md5_final()
        self.assertEqual(result, self.expectedResultInputEmpty)


if __name__ == "__main__":
    unittest.main()
