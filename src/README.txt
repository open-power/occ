# On Chip Controller (OCC)

#=======================
# I. OBTAIN THE SOURCE
#=======================

To obtain the source code, you must use Git:

git clone https://github.com/open-power/occ.git

#=============================
# II. BUILD THE BINARY IMAGE
#=============================

1. Obtain a current version of the GNU powerpc cross compiler

2. Goto the src/ directory of the Git repository

3. Build the binary files: make all

4. Combine the binaries: make combineImage
NOTE: This creates the OCC binary: image.bin

5. Create the string file for tracing: make tracehash
NOTE: This creates the debug trace hash file: occStringFile

6. Repackage the PNOR to contain modified OCC image:
op-build occ-rebuild openpower-pnor-rebuild

#=====================
# III. DOCUMENTATION
#=====================

Documentation can be found in the docs project of the open-power repository.
https://github.com/open-power/docs/tree/master/occ

#==============
# IV. LICENSE
#==============

See the LICENSE file.
