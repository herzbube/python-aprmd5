#!/bin/bash

MYNAME="$(basename "$0")"
LOGFILE="$MYNAME.log"
PYTHON_VERS_DEFAULT="system fink 2.6 3.1"

if test $# -ge 1 -a \( "$1" = "-h" -o "$1" = "--help" \); then
  cat << EOF
Usage:
  ./$MYNAME
  ./$MYNAME ver1 [ver2 ...]
  PYTHON_VERS="ver1 [ver2 ...]" ./$MYNAME system
  ./$MYNAME -h|--help

$MYNAME is a helper script that runs one build and test cycle per specified
Python version. $MYNAME probably only works on Mac OS X.

Python versions must be specified as a whitespace separated list. Either use
command line arguments, or place the version list in the environment variable
PYTHON_VERS. If neither command line nor environment variable specify any
Python versions, $MYNAME uses the following default versions:

  $PYTHON_VERS_DEFAULT

How Python versions are interpreted:
- system: The system's version of Python. The binary is expected to be
  present in /usr/bin/python.
- path: The first version of Python available from the PATH. The binary
  used is determined by this command: "which python".
- fink: Python installed through fink. The binary is expected to be
  present in /sw/bin. The first binary found whose name matches an
  acceptable pattern is used.
- anything else: Framework version of Python. The binary is expected to be
  present in /Library/Frameworks/Python.framework/Versions/\$VER/bin/python\$VER.

$MYNAME writes the result of all builds and unit test runs to a log file
for later inspection. The log file is removed if it exists when $MYNAME
is started.

Exit codes:
 0 if all builds and tests were successful
 1 if one or more build and/or test failed; a Python binary that could not be
   found does not count as a failure
EOF
  exit
fi

# Determine which Python versions have been specified
if test $# -gt 0; then
  PYTHON_VERS="$@"
elif test -n "$PYTHON_VERS"; then
  :  # no-op
else
  PYTHON_VERS="system fink 2.6 3.1"
fi
echo "Specified Python versions: $PYTHON_VERS"

unset AT_LEAST_ONE_BUILD_OR_TEST_FAILED
rm -f "$LOGFILE"
for PYTHON_VER in $PYTHON_VERS; do
  echo "Build and test started for Python version '$PYTHON_VER'"

  # Lookup Python binary to use
  if test "$PYTHON_VER" = "system"; then
    PYTHON_LOC="/usr/bin"
    PYTHON_BIN="$PYTHON_LOC/python"
  elif test "$PYTHON_VER" = "path"; then
    PYTHON_LOC="PATH"
    PYTHON_BIN="$(which python)"
  elif test "$PYTHON_VER" = "fink"; then
    PYTHON_LOC="/sw/bin"
    for PYTHON_BIN in $(ls /sw/bin/python* 2>/dev/null); do
      # Accept any of the following patterns
      case "$PYTHON_BIN" in
        /sw/bin/python)            break ;;
        /sw/bin/python[0-9].[0-9]) break ;;
        *) ;;
      esac
      # None of the files (if there were any) matched any of the
      # accepted patterns -> give up and unset PYTHON_BIN so that
      # the error message further down is triggered
      unset PYTHON_BIN
    done
  else
    PYTHON_LOC="/Library/Frameworks/Python.framework/Versions/$PYTHON_VER/bin"
    PYTHON_BIN="$PYTHON_LOC/python$PYTHON_VER"
  fi

  # Evaluate result of Python binary lookup
  if test -z "$PYTHON_BIN" -o ! -x "$PYTHON_BIN"; then
    echo "  No Python binary found in $PYTHON_LOC"
    continue
  else
    PYTHON_VER_ID="$("$PYTHON_BIN" --version 2>&1)"
    echo "  Using Python binary $PYTHON_BIN (identifies itself as '$PYTHON_VER_ID')"
  fi

  echo "  Cleaning up from previous build"
  rm -rf build src/packages/aprmd5.so
  printf "  Building the extension... "
  "$PYTHON_BIN" setup.py build_ext >>"$LOGFILE" 2>&1
  if test $? -eq 0; then
    echo "success"
  else
    echo "failed"
    AT_LEAST_ONE_BUILD_OR_TEST_FAILED=1
    # There's no point in running unit tests if the build failed
    continue
  fi
  printf "  Running unit tests... "
  "$PYTHON_BIN" setup.py test >>"$LOGFILE" 2>&1
  if test $? -eq 0; then
    echo "success"
  else
    echo "failed"
    AT_LEAST_ONE_BUILD_OR_TEST_FAILED=1
  fi
done

if test -f "$LOGFILE"; then
  echo "Details can be looked up in the log file: $LOGFILE"
fi

if test -z "$AT_LEAST_ONE_BUILD_OR_TEST_FAILED"; then
  exit 0
else
  exit 1
fi
