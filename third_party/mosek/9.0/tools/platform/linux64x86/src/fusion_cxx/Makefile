MSKMAJORVER=9
MSKMINORVER=0
CXX=g++ -std=c++11
LD=g++ -std=c++11
AR=ar

CFLAGS_DEBUG=-g
CFLAGS_RELEASE=-O2

CFLAGS=-O2 -fPIC
IPATHS=-I../../h -I.

libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER): fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os
	$(LD) -shared -L../../bin -Wl,-rpath-link,../../bin '-Wl,-rpath,$$ORIGIN' -o libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER) fusion.os mosektask.os BaseModel.os Debug.os IntMap.os SolverInfo.os StringBuffer.os -lmosek64

libfusion64.a:      fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os
	$(AR) r libfusion64.a fusion.os BaseModel.os Debug.os mosektask.os IntMap.os SolverInfo.os StringBuffer.os

%.os: %.cc
	$(CXX) $(IPATHS) $(CFLAGS) -c -o $@ $<

.PHONY: clean install

install: ../../bin/libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER) ../../bin/libfusion64.so

../../bin/libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER): libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER) libfusion64.a
	install libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER) ../../bin
	install libfusion64.a      ../../bin
../../bin/libfusion64.so: ../../bin/libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER)
	rm -f ../../bin/libfusion64.so && ln -s ./libfusion64.so.$(MSKMAJORVER).$(MSKMINORVER) ../../bin/libfusion64.so

clean:
	rm -rf *.os
