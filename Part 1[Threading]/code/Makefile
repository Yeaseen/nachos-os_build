# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

MAKE = make
LPR = lpr
SH = bash

all: 
	cd threads; $(MAKE) depend
	cd threads; $(MAKE) nachos
	cd userprog; $(MAKE) depend 
	cd userprog; $(MAKE) nachos 
	cd vm; $(MAKE) depend
	cd vm; $(MAKE) nachos 
	cd filesys; $(MAKE) depend
	cd filesys; $(MAKE) nachos 
	cd network; $(MAKE) depend
	cd network; $(MAKE) nachos 
	#cd bin; make all
	cd bin; make coff2noff
	#cd test; make all

# don't delete executables in "test" in case there is no cross-compiler
clean:
	$(SH) -c "rm -f */{core,nachos,DISK,*.o,swtch.s} test/*.coff bin/{coff2flat,coff2noff,disassemble,out}"

print:
	$(SH) -c "$(LPR) Makefile* */Makefile"
	$(SH) -c "$(LPR) threads/*.h threads/*.cc threads/*.s"
	$(SH) -c "$(LPR) userprog/*.h userprog/*.cc" 
	$(SH) -c "$(LPR) filesys/*.h filesys/*.cc
	$(SH) -c "$(LPR) network/*.h network/*.cc 
	$(SH) -c "$(LPR) machine/*.h machine/*.cc
	$(SH) -c "$(LPR) bin/noff.h bin/coff.h bin/coff2noff.c"
	$(SH) -c "$(LPR) test/*.h test/*.c test/*.s"
