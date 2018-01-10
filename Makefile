# Unix Makefile for Glulxe.

# To use this, you must set three variables. GLKINCLUDEDIR must be the 
# directory containing glk.h, glkstart.h, and the Make.library file.
# GLKLIBDIR must be the directory containing the library.a file.
# And GLKMAKEFILE must be the name of the Make.library file. Two
# sets of values appear below; uncomment one of them and change the
# directories appropriately.

GLK = emglken

GLKINCLUDEDIR = ../$(GLK)
GLKLIBDIR = ../$(GLK)
GLKMAKEFILE = Make.$(GLK)

# Pick a C compiler.
CC = emcc \
	-O3

LINK_OPTS = \
	--js-library $(GLKINCLUDEDIR)/library.js \
	-s EMTERPRETIFY=1 \
	-s EMTERPRETIFY_ASYNC=1 \
	-s EMTERPRETIFY_SYNCLIST='"@$(GLKINCLUDEDIR)/emglken_synclist.json"' \
	-s EMTERPRETIFY_WHITELIST='"@whitelist.json"' \
	-s EXPORTED_FUNCTIONS='["_emglulxeen"]' \
	-s MODULARIZE=1

OPTIONS = -Wall -Wmissing-prototypes -Wstrict-prototypes -Wno-unused -DOS_UNIX

# Locate the libxml2 library. You only need these lines if you are using
# the VM_DEBUGGER option. If so, uncomment these and set appropriately.
#XMLLIB = -L/usr/local/lib -lxml2
#XMLLIBINCLUDEDIR = -I/usr/local/include/libxml2

include $(GLKINCLUDEDIR)/$(GLKMAKEFILE)

CFLAGS = $(OPTIONS) -I$(GLKINCLUDEDIR) $(XMLLIBINCLUDEDIR)
LIBS = -L$(GLKLIBDIR) $(GLKLIB) $(LINKLIBS) -lm $(XMLLIB)

OBJS = main.o files.o vm.o exec.o funcs.o operand.o string.o glkop.o \
  heap.o serial.o search.o accel.o float.o gestalt.o osdepend.o \
  profile.o debugger.o emglulxeen.o

all: glulxe-core.js glulxe-profiler-core.js

# Set up to build twice with and without the profiler
# From https://stackoverflow.com/a/19744628/2854284
PROFILEROBJ = $(patsubst %.o, profiler/%.o, $(OBJS))
NOPROFILEROBJ = $(patsubst %.o, noprofiler/%.o, $(OBJS))

profiler/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
noprofiler/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

profiler:
	mkdir -p profiler
noprofiler:
	mkdir -p noprofiler

glulxe-core.js: LINK_OPTS += -s EMTERPRETIFY_FILE='"glulxe-core.js.bin"'
glulxe-profiler-core.js: CFLAGS += -DVM_PROFILING
glulxe-profiler-core.js: LINK_OPTS += -s EMTERPRETIFY_FILE='"glulxe-profiler-core.js.bin"'


glulxe-core.js: $(NOPROFILEROBJ) $(GLKINCLUDEDIR)/Make.$(GLK) $(GLKINCLUDEDIR)/libemglken.a $(GLKINCLUDEDIR)/library.js
	$(CC) $(OPTIONS) $(LINK_OPTS) -o $@ $(NOPROFILEROBJ) $(LIBS)

glulxe-profiler-core.js: $(PROFILEROBJ) $(GLKINCLUDEDIR)/Make.$(GLK) $(GLKINCLUDEDIR)/libemglken.a $(GLKINCLUDEDIR)/library.js
	$(CC) $(OPTIONS) $(LINK_OPTS) -o $@ $(PROFILEROBJ) $(LIBS)


$(PROFILEROBJ): glulxe.h emglulxeen.h | profiler
$(NOPROFILEROBJ): glulxe.h emglulxeen.h | noprofiler

%/exec.o %/operand.o: opcodes.h
%/gestalt.o: gestalt.h


clean:
	rm -f *~ *.o glulxe-core.js* glulxe-profiler-core.js*
	rm -rf noprofiler profiler