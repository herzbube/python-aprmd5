#!/bin/bash

# Setup a few variables
MYNAME="$(basename "$0")"
LOGFILE="$MYNAME.log"
BUILD_FOLDER="build"
BUILT_EXTENSION_FILE_NAME="aprmd5.so"
TMP_FOLDER="/tmp/$MYNAME.$$"
INSTALL_FOLDER="$TMP_FOLDER/install"
# Unset these to skip the corresponding steps
TEST_STEP=1
INSTALL_STEP=1
unset HELP PYTHON_VERS
PYTHON_VERS_DEFAULT="system fink 2.6 3.1"

# Process arguments
while test $# -gt 0; do
  OPTION="$1"
  case "$OPTION" in
    -h|--help)
      HELP=1
      ;;
    *)
      PYTHON_VERS="$PYTHON_VERS $OPTION"
      ;;
  esac
  shift 1
done

# Test if help should be displayed
if test -n "$HELP"; then
  cat << EOF
Usage:
  $MYNAME -h|--help
  $MYNAME
  $MYNAME ver1 [ver2 ...]
  PYTHON_VERS="ver1 [ver2 ...]" $MYNAME

$MYNAME is a helper script that runs one cycle consisting of a build,
test and install step per specified Python version.

Python versions must be specified as a whitespace separated list. Either use
command line arguments, or place the version list in the environment variable
PYTHON_VERS. If neither command line nor environment variable are present,
$MYNAME uses the following default versions:

  $PYTHON_VERS_DEFAULT

$MYNAME writes the result of all the cycles it runs to a log file for
later inspection. The log file is removed if it exists when $MYNAME
is started.

Note about how Python versions are interpreted:
- system: The system's version of Python. The binary is expected to be present
  in /usr/bin/python.
- path: The first version of Python available from the PATH. The binary used
  is determined by this command: "which python".
- fink: Python installed through fink (a packaging project on Mac OS X). The
  binary is expected to be present in /sw/bin. The first binary found whose
  name matches an acceptable pattern is used.
- anything else: A named version of Python. On Linux, the binary is expected to
  be present in /usr/bin/python\$VER. On Mac OS X, the binary is expected to be
  present in /Library/Frameworks/Python.framework/Versions/\$VER/bin/python\$VER.

Exit codes:
 0 if all steps in all cycles were successful
 1 if one or more step in any cycle failed; a Python binary that could not be
   found does not count as a failure
EOF
  exit
fi

# Process argument defaults
if test -z "$PYTHON_VERS"; then
  PYTHON_VERS="$PYTHON_VERS_DEFAULT"
fi

# Creating temporary folder (must not exist)
mkdir -p "$TMP_FOLDER"
if test $? -ne 0; then
  echo "Failed to create temp folder $TMP_FOLDER"
  exit 1
fi

# Print preliminary information
echo "Python versions: $PYTHON_VERS"
echo "Temp folder: $TMP_FOLDER"

# Cycle through specified Python versions
unset AT_LEAST_ONE_STEP_FAILED
rm -f "$LOGFILE"
for PYTHON_VER in $PYTHON_VERS; do
  echo "Cycle started for Python version '$PYTHON_VER'"

  # Lookup Python binary to use
  unset PYTHON_LOC
  unset PYTHON_BIN
  if test "$PYTHON_VER" = "system"; then
    PYTHON_LOC="/usr/bin"
    PYTHON_BIN="$PYTHON_LOC/python"
  elif test "$PYTHON_VER" = "path"; then
    PYTHON_LOC="PATH"
    PYTHON_BIN="$(which python)"
  elif test "$PYTHON_VER" = "fink"; then
    PYTHON_LOC="/sw/bin"
    if test -d "$PYTHON_LOC"; then
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
    fi
  else
    case "$(uname)" in
      Darwin)
        PYTHON_LOC="/Library/Frameworks/Python.framework/Versions/$PYTHON_VER/bin"
        ;;
      Linux)
        PYTHON_LOC="/usr/bin"
        ;;
      *)
        PYTHON_LOC="/usr/bin"
        ;;
    esac
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

  echo "  Cleaning up from previous cycle"
  rm -rf "$BUILD_FOLDER" "$INSTALL_FOLDER"

  printf "  Building the extension... "
  "$PYTHON_BIN" setup.py build_ext >>"$LOGFILE" 2>&1
  if test $? -eq 0; then
    echo "success"
  else
    echo "failed"
    AT_LEAST_ONE_STEP_FAILED=1
    # There's no point in running unit tests if the build failed
    continue
  fi

  if test -n "$TEST_STEP"; then
    printf "  Running unit tests... "
    BUILT_EXTENSION_FILE_PATH="$(find "$BUILD_FOLDER" -name "$BUILT_EXTENSION_FILE_NAME" 2>/dev/null)"
    if test -z "$BUILT_EXTENSION_FILE_PATH"; then
      echo "failed (could not find built extension)"
      AT_LEAST_ONE_STEP_FAILED=1
      continue
    fi
    cp "$BUILT_EXTENSION_FILE_PATH" "$TMP_FOLDER" >/dev/null 2>&1
    if test $? -ne 0; then
      echo "failed (could not copy built extension)"
      AT_LEAST_ONE_STEP_FAILED=1
      continue
    fi
    PYTHONPATH="$TMP_FOLDER" "$PYTHON_BIN" setup.py test >>"$LOGFILE" 2>&1
    if test $? -eq 0; then
      echo "success"
    else
      echo "failed"
      AT_LEAST_ONE_STEP_FAILED=1
      continue
    fi
  fi

  if test -n "$INSTALL_STEP"; then
    printf "  Testing installation... "
    "$PYTHON_BIN" setup.py install "--home=$INSTALL_FOLDER" >>"$LOGFILE" 2>&1
    if test $? -eq 0; then
      echo "success"
    else
      echo "failed"
      AT_LEAST_ONE_STEP_FAILED=1
      continue
    fi
  fi
done
rm -rf "$TMP_FOLDER"

if test -f "$LOGFILE"; then
  echo "Details can be looked up in the log file: $LOGFILE"
fi

if test -z "$AT_LEAST_ONE_STEP_FAILED"; then
  exit 0
else
  exit 1
fi
