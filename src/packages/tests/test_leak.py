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


"""Rather crude unit tests for memory leaks in aprmd5.md5.

This unit test module only works on systems that have a couple of basic command
line utilities such as ps and tail. __init__.py only includes this unit test
module if os.name == "posix".
"""


# PSL
import unittest
import os

# python-aprmd5
from aprmd5 import md5
import tests   # import stuff from __init__.py (e.g. tests.python2)


def mem():
    """Return the current process' memory usage in KB

    The resolution that this function can measure might not be as accurate as
    1 KB, i.e. two invocations of this method might yield the same result even
    though the process' memory usage has in fact increased by 1 KB. The reason
    for this is that this method uses the ps command line utility, and that
    utility may see only increases of one memory page (e.g. 4 KB on Mac OS X).
    """
    return int(os.popen('ps -p %d -o rss | tail -1' % os.getpid()).read())


class MemoryLeakTest(unittest.TestCase):

    def setUp(self):
        if tests.python2:
            self.inputNormal = "foo"
        else:
            # Convert into bytes. We can use UTF-8 because we know that this
            # file, and therefore the literal "foo", is UTF-8 encoded.
            self.inputNormal = "foo".encode("utf-8")
        self.expectedHexdigestInputNormal = "acbd18db4cc2f85cedef654fccc4a4d8"

    def testRefCountCreate(self):
        """Tests if reference count is set correctly on creation"""
        # Not sure why, but on my Mac box I have to do this, otherwise tests
        # fail with memory usage difference of 4 KB. Probably some internals
        # of Python claim memory when the first md5 object is created...
        md5()
        # The number of md5 objects to create. Should be high enough so that
        # the cumulative memory leak (if it exists) is greater than a memory
        # page size (see limitations of mem()).
        objectCount = 10000
        # Get memory after all objects that remain in use have been created.
        # After this statement only objects must be created that are expected
        # to be destroyed before the next invocation of mem().
        memory = mem()
        while objectCount > 0:
            objectCount -= 1
            # Store no references. If the C implementation handles reference
            # counts correctly, it can be expected that the object will be
            # destroyed immediately.
            md5()
        self.assertEqual(memory, mem())

    def testRefCountCopy(self):
        """Tests if reference count is set correctly on md5.copy()"""
        md5()
        objectCount = 10000
        memory = mem()
        while objectCount > 0:
            objectCount -= 1
            md5().copy()
        self.assertEqual(memory, mem())

    def testRefCountHash(self):
        """Tests if no memory leaks occur when performing a hash operation"""
        m = md5(self.inputNormal)
        objectCount = 10000
        memory = mem()
        while objectCount > 0:
            objectCount -= 1
            m = md5(self.inputNormal)
            self.assertEqual(m.hexdigest(), self.expectedHexdigestInputNormal)
        self.assertEqual(memory, mem())


if __name__ == "__main__":
    unittest.main()
