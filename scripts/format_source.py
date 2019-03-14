#!/usr/bin/python

"""
*******************************************************************************

Copyright (c) 2016-2019 VMware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************
"""

"""

format_source.py

   Formats the source per the coding standards and clang-format.

"""

from optparse import OptionParser
from datetime import datetime
import os
import platform
import subprocess
import sys


def FormatSourceTree(treeRoot, clangPath, includeDirs, fileExtensions):
   absPath = os.path.abspath(treeRoot)
   sourceFiles = os.listdir(absPath)
   clangFormatPath = os.path.join(clangPath, "clang-format")

   for fileName in sourceFiles:
      filePath = os.path.join(absPath, fileName)

      if (os.path.isfile(filePath) and
          filePath != os.path.abspath(__file__)):

         for ext in fileExtensions:
            if not filePath.endswith(ext):
               continue

            print "Formatting: %s" % (filePath)
            os.system("%s -i %s" % (clangFormatPath, filePath))
      elif (os.path.isdir(filePath) and
            fileName != "external" and
            fileName != "build"):
         FormatSourceTree(filePath, clangPath, includeDirs,
                          fileExtensions)


if __name__ == "__main__":
   startTime = datetime.now()

   print "*********************************************************************************"
   print "Formatting source tree using clang-format"
   print ""
   print "os.name = %s" % (os.name)
   print "platform.system() = %s" % (platform.system())
   print "sys.platform = %s" % (sys.platform)
   print ""
   print "Time = %s" % (startTime)
   print "********************************************************************************"

   #
   # Handle command line options for the parser
   #
   parser = OptionParser()

   parser.add_option("-c", "--clangPath", dest="CLANG_PATH",
                     default=None,
                     help="Specifies the path for clang.")
   parser.add_option("-t", "--sourceTree", dest="SOURCE_TREE",
                     default=None,
                     help="Specifies the path for the source tree to be "
                          "formatted.")
   parser.add_option("-i", "--includeDirs", dest="INCLUDE_DIRS",
                     default=None,
                     help="Specifies the include directories for the source tree.")
   parser.add_option("-x", "--extensions", dest="FILE_EXTENSIONS",
                     default=".c,.cpp,.h,.hpp,.m,.cc,.java,.js",
                     help="Specifies a comma delimited list of file extensions.")
   (options, args) = parser.parse_args()

   #
   # Check for the required json argument file
   #
   clangPath = os.path.abspath(options.CLANG_PATH)
   includeDirs = os.path.abspath(options.INCLUDE_DIRS)

   if (options.CLANG_PATH is None or options.CLANG_PATH == "" or
       not os.path.exists(clangPath)):
      print "ERROR: You must supply a directory containing clang-format"
      print "       e.g. ../build/external/spirv-llvm/bin"
      sys.exit(1)

   print "clang path: %s" % (clangPath)
   print "Source tree: %s" % (os.path.abspath(options.SOURCE_TREE))
   print "Include directories: %s" % (includeDirs)
   print "File extensions: %s" % (options.FILE_EXTENSIONS)
   print ""

   FormatSourceTree(options.SOURCE_TREE, clangPath, includeDirs,
                    options.FILE_EXTENSIONS.split(","))

   endTime = datetime.now()

   print "********************************************************************************"
   print "Start Time: %s" % (str(startTime))
   print "End Time  : %s" % (str(endTime))
   print "Elapsed   : %s" % (str(endTime - startTime))
   print ""
   print "Exiting format_source.py..."
   print "********************************************************************************"
   sys.exit(0)
