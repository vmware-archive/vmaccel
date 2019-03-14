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
gen_formats_enum.py

   Generates the formats enumerants given a formats template JSON file.

"""

from optparse import OptionParser
from datetime import datetime
import json
import os
import platform
import subprocess
import sys


if __name__ == "__main__":
   startTime = datetime.now()

   print "*********************************************************************************"
   print "Generating formats enumerants CSV"
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

   parser.add_option("-i", "--input", dest="INPUT_JSON",
                     default=None,
                     help="Specifies the formats template JSON file.")
   parser.add_option("-o", "--output", dest="OUTPUT_CSV",
                     default=None,
                     help="Specifies the path for the output comman separated file")
   (options, args) = parser.parse_args()

   #
   # Check for the required json argument file
   #
   if options.INPUT_JSON is None or options.INPUT_JSON == "":
      print "ERROR: You must supply a JSON argument file, i.e. \"gen_formats_enum.py -i formats.json\""
      sys.exit(1)

   if options.OUTPUT_CSV is None or options.OUTPUT_CSV == "":
      print "ERROR: You must supply an output CSV file, i.e. \"gen_formats_enum.py -o enums.h\""
      sys.exit(1)

   print "JSON input: %s" % (options.INPUT_JSON)
   print "CSV output: %s" % (options.OUTPUT_CSV)

   try:
      formats = json.load(open(options.INPUT_JSON, 'r'))
   except Exception as e:
      print "Unable to load JSON file %s" % (options.INPUT_JSON)
      sys.exit(1)

   try:
      f = open(options.OUTPUT_CSV, 'w')
   except Exception as e:
      print "Unable to open output CSV file %s" % (options.OUTPUT_CSV)
      sys.exit(1)

   numFormats = 0

   #
   # Loop through each format and derive the proper name
   #
   for format in formats:
      components = format['components']

      bitDesc = ""

      for comp in list(components):
         compKey = "%s_bits" % (comp)

         numBits = format[compKey]
         bitDesc += "%s%s" % (comp.upper(), str(numBits))

      types = format['types'].split(',')

      for type in types:
         formatName = "VMACCEL_FORMAT_%s_%s" % (bitDesc, type.upper())
 
         f.write("%s,\n" % (formatName))

         numFormats += 1

   f.close()

   endTime = datetime.now()

   print "********************************************************************************"
   print "numFormats = %s" % (str(numFormats))
   print ""
   print "Start Time: %s" % (str(startTime))
   print "End Time  : %s" % (str(endTime))
   print "Elapsed   : %s" % (str(endTime - startTime))
   print ""
   print "Exiting gen_formats_enum.py..."
   print "********************************************************************************"
   sys.exit(0)
