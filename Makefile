# Unix Makefile for Glulxe.

# To use this, you must set three variables. GLKINCLUDEDIR must be the 
# directory containing glk.h, glkstart.h, and the Make.library file.
# GLKLIBDIR must be the directory containing the library.a file.
# And GLKMAKEFILE must be the name of the Make.library file. Two
# sets of values appear below; uncomment one of them and change the
# directories appropriately.

GLKINCLUDEDIR = ../cheapglk
GLKLIBDIR = ../cheapglk
GLKMAKEFILE = Make.cheapglk

#GLKINCLUDEDIR = ../glkterm
#GLKLIBDIR = ../glkterm
#GLKMAKEFILE = Make.glkterm

#GLKINCLUDEDIR = ../xglk
#GLKLIBDIR = ../xglk
#GLKMAKEFILE = Make.xglk

# Pick a C compiler.
#CC = cc
CC = gcc

OPTIONS = -g -Wall -Wno-unused -DOS_UNIX

include $(GLKINCLUDEDIR)/$(GLKMAKEFILE)

CFLAGS = $(OPTIONS) -I$(GLKINCLUDEDIR)
LIBS = -L$(GLKLIBDIR) $(GLKLIB) $(LINKLIBS) 

OBJS = main.o files.o vm.o exec.o funcs.o operand.o string.o glkop.o \
  serial.o osdepend.o

all: glulxe

glulxe: $(OBJS) unixstrt.o
	$(CC) $(OPTIONS) -o glulxe $(OBJS) unixstrt.o $(LIBS)

glulxdump: glulxdump.o
	$(CC) -o glulxdump glulxdump.o

$(OBJS): glulxe.h

exec.o operand.o: opcodes.h

clean:
	rm -f *~ *.o glulxe glulxdump

