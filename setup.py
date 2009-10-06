#!/usr/bin/env python
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
from distutils.core import setup, Extension
from distutils.cmd import Command
import unittest
import sys

# Here we define the header file that the .c source file is going to include,
# and the library file that we are going to use for linking.

# Mac OS X 10.5 (system)
aprmd5_header_filename = "<apr-1/apr_md5.h>"
aprmd5_library_filename = "aprutil-1"

# Mac OS X, provided by fink (package apr-dev)
#aprmd5_header_filename = "<apr-0/apr_md5.h>"
#aprmd5_library_filename = "aprutil-0"

# Debian lenny (package libaprutil1-dev)
#aprmd5_header_filename = "<apr-1.0/apr_md5.h>"
#aprmd5_library_filename = "aprutil-1"


# Here we define extra compiler/linker arguments.

# Mac OS X 10.5 (system)
include_dirs = None
library_dirs = None
extra_compile_args = ["-isysroot", "/"]   # add "-v" for verbose output
extra_link_args = ["-isysroot", "/"]      # add "-t" for verbose output

# Mac OS X, provided by fink (package apr-dev)
#include_dirs = ["/sw/include"]
#library_dirs = ["/sw/lib"]
#extra_compile_args = None
#extra_link_args = None

# Debian lenny (package libaprutil1-dev)
#include_dirs = None
#library_dirs = None
#extra_compile_args = None
#extra_link_args = None


# Create the Extension object.
aprmd5 = Extension("aprmd5",
                   sources = ["src/aprmd5.c"],
                   define_macros = [("APR_MD5_HEADER_FILENAME", aprmd5_header_filename)],
                   libraries = [aprmd5_library_filename],
                   include_dirs = include_dirs,
                   library_dirs = library_dirs,
                   extra_compile_args = extra_compile_args,
                   extra_link_args = extra_link_args)


# Extend search path for packages and modules. This is required for finding the
# "tests" package and its modules.
PACKAGES_BASEDIR = "src/packages"
sys.path.append(PACKAGES_BASEDIR)

class test(Command):
    """Implements a distutils command to execute automated tests.

    To run the command, a user must type one of the following:
      ./setup.py test                          # run all tests
      ./setup.py test --suite=tests.test_foo   # run tests from only one module

    The class name is the same as the command name string used in the 'cmdclass'
    dictionary passed to the setup() function further down. The reason for this
    is that, unfortunately, 'python setup.py test --help' will print out the
    class name instead of the name used in the dictionary (or the 'command_name'
    attribute defined in this class).
    """

    # This must be a class attribute; it is used by
    # "python setup.py --help-commands"
    description = "execute automated tests"

    # Options must be defined in a class attribute. The attribute value is a
    # list of tuples. Each tuple defines an option and must contain 3 values:
    # long option name, short option name, and a description to print with
    # --help. An option that should have an argument must have the suffix "=".
    # Each option defined in user_options must have a data attribute with a
    # name that corresponds to the long name of the option. For instance, an
    # option "--foo-bar" requires an attribute "foo_bar". If the user has
    # specified the option, a value is set to the data attribute. If the
    # option has no argument, the attribute value is set to 1. If the option
    # has an argument, the attribute value is set to the argument value.
    user_options = [("suite=", "s", "run test suite for a specific module [default: run all tests]")]

    def __init__(self, dist):
        # This data attribute is returned by Command.get_command_name()
        self.command_name = "test"
        Command.__init__(self, dist)

    def initialize_options(self):
        # The default value is a callable defined in tests.__init__.py. The user
        # must specify something like this: "--suite tests.test_algorithm"
        self.suite = "tests.allTests"   

    def finalize_options(self):
        pass

    def run(self):
        tests = unittest.defaultTestLoader.loadTestsFromName(self.suite)
        testRunner = unittest.TextTestRunner(verbosity = 1)
        testRunner.run(tests)


setup(
      # List extension modules
      ext_modules= [aprmd5],
      # Add a command named "test". The name string in the dict is also used by
      # "python setup.py --help-commands", but not by "python setup.py test -h"
      cmdclass = { "test" : test },
      # Meta-data
      name="python-aprmd5",
      version="0.1",
      url="http://www.herzbube.ch/python-aprmd5",
      author="Patrick Näf",
      author_email="herzbube@herzbube.ch",
      description="Python interface to the MD5 routines of libaprutil",
      long_description=
"""
The python-aprmd5 project provides the Python module "aprmd5", which in turn is
an interface to the MD5 routines of the Apache Portable Runtime (APR) Utility
Library (libaprutil).

The main purpose of python-aprmd5 is to expose a single function,
apr_md5_encode(), from among libaprutil's MD5 routines. That function is used to
generate salted crypt-style hashes using a version of the MD5 hash algorithm
that was modified for Apache. The resulting hashes always start with the prefix
characters "$apr1$" in order to distinguish them from the result of various
other crypt() variants, which use other prefixes. For instance, for input "foo"
and salt "mYJd83wW", apr_md5_encode() produces the following hash:

  $apr1$mYJd83wW$IO.6aK3G0d4mHxcImhPX50

Hashes like this are typically generated by the command line utility htpasswd,
which is part of the Apache HTTP server project. The hashes encrypt user
passwords that are used by the Apache HTTP server for basic user authentication.

In an attempt to earn its name, python-aprmd5 tries to expose most of the other
functions of libaprutil's MD5 routines as well. It doesn't try too hard, though,
since those functions are mostly concerned with the original, unmodified MD5
algorithm, and access to that algorithm can be easily obtained through the
Python Standard Library module "hashlib".
""")
