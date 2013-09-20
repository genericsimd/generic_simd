#Developer Guide


##Source code structure

The package contains the following structures:
- <b>docs</b> Generate html docs through doxygen
- <b>examples</b> Examples using the library 
- <b>include</b> The library source code
- <b>tests</b> Unit tests, test library implementation 

When adding a new target platform, need to add <platform>.h to include/ and may need to modify makefiles so that the new platform can be tested and will run with existing examples.

##Run unit tests

Unit tests are in tests directory. The current unit test covers power_vsx4.h, generic4.h, generic8,h and sse4.h

Please download googletest framework first from https://code.google.com/p/googletest/, and unzip it into "tests/gtest-1.6.0" dirctory.
Or you can unzip it to where you want, and modify the "GTEST_DIR" value in tests/Makefile.

Then you can run the test
```bash
$ cd tests
$ make clean
$ make {vsx4|sse4|generic4|generic8}     # build/run unit tests for target SIMD ISA
```
The test app will test vsx4, generic4 and sse4 interfaces, and generate the report.


##Generate the documentation

We use doxygen to generate documentations. The input files for doxygen is under <gsimd_path>/docs/. To update the documentation, either modigy the *.txt files or doxygen annotations in the library source codes.

To publish new documentations, you need to go through the following steps:

1. Make sure you have doxygen installed

2. Checkout the gh-pages branch of your project to docs/gh-pages.github
```bash
$ add docs/gh-pages.github to .gitignore
$ cd docs
# clone the project repo to docs/gh-pages.github
$ git clone https://github.com/pengwuibm/generic_simd.git gh-pages.github
$ cd gh-pages.github
$ git checkout gh-pages   # switch to the gh-pages branch of the project repo
```
  
3. Generate new doxygen pages and copy into gh-pages.github
```bash
$ cd docs
$ make         # generate documentation into docs/html
$ make gitpub  # copy docs/html into docs/gh-pages.github
$ cd gh-pages.github
$ git commit -a # checkin new documentation to github
$ git status   # to check if there is any new file (untracked)
$ manually add any new file "git add ..." and "git commit"
$ git push     # push to github
```
  Note: it may take 10 minutes before the new pages appear on http://pengwuibm.github.io/generic_simd
