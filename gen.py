#!/usr/bin/python
#
# trans.py, translate netlist to suitable format
# Usage: python trans.py <input nexlist file> [output result file (option)]
#

from __future__ import print_function
import re
import sys

#
# start main program
#
def main():
	data = ""
	# try to open read file
	if len(sys.argv) <= 1:
		print ("gen.py <configure file> [<header file>]")
		sys.exit()
	try:
		ifd = open(sys.argv[1], "r")
	except IOError:
		print ("Could not open read file \"", sys.argv[1],"\"")
		sys.exit()

	# try to open write file
	try:
		ofd = open(sys.argv[2], "w+")
	except:
		pass

	# repeat read line from file
	s=""
	try:
		ofd.write("//\n// system configuration\n//\n")
	except:
		None
		
	for line in ifd:
		l = line.replace('\n', '').strip(' ').split('#')
		if l[0]:
			l = l[0].replace('?=',':').replace(':=',':').strip()
			l = l.split(':')
			if l[1]=="1":
				s = "#define " + l[0] + "\n"
				# try to write data into file
				try:
					ofd.write(s)
				except:
					print(s, end='')
		s =""
	# close read file
	ifd.close()

	# try to close write file
	try:
		ofd.close
	except:
		pass
	print("Generate header file Done.")
#
# end main progam
#
if __name__ == '__main__':
	main()