#!/usr/bin/env python3

import sys
import shutil
import os
import hashlib

testid     = sys.argv[1]
md5        = sys.argv[2]
executable = sys.argv[3]
srcfile    = sys.argv[4]
parameters = sys.argv[5]

testfile   = 'test_file.' + testid
print('Copying', srcfile, 'to', testfile)
shutil.copy(srcfile, testfile)

cmd = executable + ' ' + parameters + ' ' + testfile
print('Executing command:', cmd)
rv = os.system(cmd)
print('Return value:', rv)

if rv != 0:
    os.remove(testfile)
    sys.exit(1)

print('Calculating md5 hash of', testfile)
hash = hashlib.md5()
with open(testfile, "rb") as f:
    for block in iter(lambda: f.read(4096), b""):
        hash.update(block)
calc_md5 = hash.hexdigest()
os.remove(testfile)

print('Required hash: ' + md5 + ', calculated: ' + calc_md5)
sys.exit(0 if md5 == calc_md5 else 1)
