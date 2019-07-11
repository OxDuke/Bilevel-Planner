MSKMAJORVER=9
MSKMINORVER=0
CXX=clang++ -std=c++11
LD=clang++ -std=c++11
AR=ar

CFLAGS=-O2 -fPIC
IPATHS=-I../../h -I.

libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib: fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os
	$(LD) -headerpad_max_install_names -shared -L../../bin -o libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib fusion.os BaseModel.os Debug.os mosektask.os  IntMap.os SolverInfo.os StringBuffer.os -lmosek64
	install_name_tool -change libmosek64.$(MSKMAJORVER).$(MSKMINORVER).dylib $(shell cd ../../bin; pwd)/libmosek64.$(MSKMAJORVER).$(MSKMINORVER).dylib libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib

libfusion64.a: fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os
	$(AR) r libfusion64.a fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os

%.os: %.cc
	$(CXX) $(IPATHS) $(CFLAGS) -c -o $@ $<

.PHONY: clean install

install: ../../bin/libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib ../../bin/libfusion64.dylib

../../bin/libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib: libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib libfusion64.a
	install libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib ../../bin
	install libfusion64.a      ../../bin

../../bin/libfusion64.dylib: ../../bin/libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib
	rm -f ../../bin/libfusion64.dylib && ln -s ./libfusion64.$(MSKMAJORVER).$(MSKMINORVER).dylib ../../bin/libfusion64.dylib

clean:
	rm -rf *.os
