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


"""Unit tests for aprmd5.md5"""

# PSL
import unittest

# python-aprmd5
from aprmd5 import md5
import tests   # import stuff from __init__.py (e.g. tests.python2)


class MD5Test(unittest.TestCase):
    """Exercise aprmd5.md5"""

    def setUp(self):
        if tests.python2:
            self.inputNormal = "foo"
            self.inputEmpty = ""
        else:
            # Convert into bytes. We can use UTF-8 because we know that this
            # file, and therefore the literal "foo", is UTF-8 encoded.
            self.inputNormal = "foo".encode("utf-8")
            self.inputEmpty = bytes()
        self.expectedHexdigestInputNormal = "acbd18db4cc2f85cedef654fccc4a4d8"
        self.expectedDigestInputNormal = b'\xac\xbd\x18\xdbL\xc2\xf8\\\xed\xefeO\xcc\xc4\xa4\xd8'
        self.expectedHexdigestInputEmpty = "d41d8cd98f00b204e9800998ecf8427e"
        self.expectedHexdigestInputTwice = "fdba98970961edb29f88241b9d99d890"

    def testCreate(self):
        m = md5()
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputEmpty)

    def testCreateInitInput(self):
        m = md5(self.inputNormal)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputNormal)

    def testCreateKeywordInitInput(self):
        m = md5(input = self.inputNormal)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputNormal)

    def testCreateInitInputIsEmpty(self):
        m = md5(self.inputEmpty)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputEmpty)

    def testCreateInitInputIsNone(self):
        input = None
        self.assertRaises(TypeError, md5, input)

    def testCreateTwice(self):
        m1 = md5()
        m2 = md5()
        self.assertNotEqual(m1, m2)

    def testDelete(self):
        # There's not much that we can test besides the bare fact that an md5
        # object can be deleted without any error
        m = md5()
        del m

    def testUpdate(self):
        m = md5()
        m.update(self.inputNormal)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputNormal)

    def testUpdateInputIsEmpty(self):
        m = md5()
        m.update(self.inputEmpty)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputEmpty)

    def testUpdateInputIsNone(self):
        m = md5()
        input = None
        self.assertRaises(TypeError, m.update, input)

    def testUpdateTwice(self):
        m = md5()
        m.update(self.inputNormal)
        m.update(self.inputNormal)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputTwice)

    def testDigest(self):
        m = md5()
        m.update(self.inputNormal)
        digest = m.digest()
        self.assertEqual(digest, self.expectedDigestInputNormal)

    def testHexdigest(self):
        # This test is somewhat pointless, the hexdigest() method has already
        # been thoroughly tested by other functions. We still perform this
        # test so that hexdigest() has at least one explicit test.
        m = md5()
        m.update(self.inputNormal)
        hexdigest = m.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputNormal)

    def testHexdigestTwice(self):
        m = md5()
        m.update(self.inputNormal)
        hexdigest1 = m.hexdigest()
        hexdigest2 = m.hexdigest()
        self.assertEqual(hexdigest1, self.expectedHexdigestInputNormal)
        self.assertEqual(hexdigest2, self.expectedHexdigestInputNormal)

    def testUpdateAfterHexdigest(self):
        m = md5()
        m.update(self.inputNormal)
        hexdigest1 = m.hexdigest()
        m.update(self.inputNormal)
        hexdigest2 = m.hexdigest()
        self.assertEqual(hexdigest1, self.expectedHexdigestInputNormal)
        self.assertEqual(hexdigest2, self.expectedHexdigestInputTwice)

    def testCopy(self):
        m1 = md5(self.inputNormal)
        m2 = m1.copy()
        self.assertNotEqual(m1, m2)
        hexdigest1 = m1.hexdigest()
        hexdigest2 = m2.hexdigest()
        self.assertEqual(hexdigest1, self.expectedHexdigestInputNormal)
        self.assertEqual(hexdigest2, self.expectedHexdigestInputNormal)
        m1.update(self.inputNormal)
        hexdigest1 = m1.hexdigest()
        hexdigest2 = m2.hexdigest()
        self.assertEqual(hexdigest1, self.expectedHexdigestInputTwice)
        self.assertEqual(hexdigest2, self.expectedHexdigestInputNormal)

    def testCopyTwice(self):
        m1 = md5(self.inputNormal)
        m2 = m1.copy()
        m3 = m1.copy()
        self.assertNotEqual(m1, m2)
        self.assertNotEqual(m1, m3)
        self.assertNotEqual(m2, m3)

    def testCopyAndDelete(self):
        m1 = md5(self.inputNormal)
        m2 = m1.copy()
        del m1
        m2.update(self.inputNormal)
        hexdigest = m2.hexdigest()
        self.assertEqual(hexdigest, self.expectedHexdigestInputTwice)

    def testDigestSize(self):
        m = md5()
        self.assertEqual(m.digest_size, 16)

    def testBlockSize(self):
        m = md5()
        self.assertEqual(m.block_size, 64)

    def testName(self):
        m = md5()
        self.assertEqual(m.name, "md5")


if __name__ == "__main__":
    unittest.main()
