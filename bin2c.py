#!/usr/bin/python

import sys

fh = open(sys.argv[1], "rb")
data = fh.read()
fh.close()

name = sys.argv[1].strip(".bin")

sys.stdout.write("const unsigned char %s_data[] = {\n" % (name))

for no, byte in enumerate(data):
	if no % 16 == 0:
		sys.stdout.write("  \"")
	sys.stdout.write("\\x%02x" % (ord(byte)))
	if no % 16 == 15:
		sys.stdout.write( "\"\n")

sys.stdout.write("\"};\n")
sys.stdout.write("const unsigned int %s_size = %d;\n" % (name, len(data)))

