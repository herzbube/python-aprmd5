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


# PSL
import unittest

# python-aprmd5
from tests import test_md5_encode
from tests import test_md5_operation
from tests import test_md5
from tests import test_password_validate


def allTests():
    """This function is used by the test command in setup.py.

    This function is a callable that can be used from anywhere by
    unittest.TestLoader.loadTestsFromName().
    """

    suite = unittest.TestSuite()
    suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(test_md5_encode))
    suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(test_md5_operation))
    suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(test_md5))
    suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(test_password_validate))
    return suite
