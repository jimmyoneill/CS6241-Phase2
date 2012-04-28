#! /usr/bin/python

import os
import sys
import subprocess

if len(sys.argv) > 1 :
	listing = os.listdir(sys.argv[1]);
	for file in listing:
		if file.endswith(".c") or file.endswith(".cpp"):
			filename = os.path.basename(file)
			bitcodeFilename = os.path.splitext(filename)[0] + ".bc"
			bitcodeLlFilename = os.path.splitext(filename)[0] + ".ll"
			clangCommand = "clang -O0 -emit-llvm -fwrapv -S " + filename + " -o " + bitcodeFilename
			optCommand = "opt -mem2reg -instnamer " + bitcodeFilename + " -o " + bitcodeLlFilename
			print("Command: " + clangCommand)
			subprocess.call(["clang", "-O0", "-emit-llvm", "-fwrapv", "-S", filename, "-o", bitcodeFilename])
			print("Command: " + optCommand)
			subprocess.call(["opt", "-mem2reg", "-instnamer", "-S", bitcodeFilename, "-o", bitcodeLlFilename])
else:
	print("Not enough arguments.")
