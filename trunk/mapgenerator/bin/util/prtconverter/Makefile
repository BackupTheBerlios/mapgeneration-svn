#*******************************************************************************
# MapGeneration Project - Creating a road map for the world.                   *
#                                                                              *
# Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
# Licensed under the Academic Free License version 2.1                         *
#******************************************************************************/

# Makefile for the MapGeneratorGUI

# Objectfiles to use for the MapAndGiude converter
objects := prtconverter.o prtconvertermain.o
	
# Directory definitions
top_builddir := ../../../
bindir := ../../../bin
srcdir := ../../../src
prtbindir = $(bindir)/util/prtconverter/
prtsrcdir = $(srcdir)/util/prtconverter/

# Variables from autoconf
CXX = /opt/gcc-4.0.0/bin/g++
CPPFLAGS = -I$(srcdir) -I$(top_builddir)  -I/opt/libraries/commoncpp2-1.2.5_gcc-4.0.0/include -g -O2 -D_GNU_SOURCE -I/opt/libraries/wxGTK-2.5.4_gcc-4.0.0/lib/wx/include/gtk2-ansi-release-2.5 -I/opt/libraries/wxGTK-2.5.4_gcc-4.0.0/include/wx-2.5 -DGTK_NO_CHECK_CASTS -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D_LARGEFILE_SOURCE=1 -DNO_GCC_PRAGMA  
CXXFLAGS =  -g -DDEBUG -O0
DEFS = -DHAVE_CONFIG_H
LDFLAGS = 
LIBS = -lodbc  -L/opt/libraries/commoncpp2-1.2.5_gcc-4.0.0/lib -lccext2 -lccgnu2 -lz -ldl -pthread -L/opt/libraries/wxGTK-2.5.4_gcc-4.0.0/lib -pthread   -L/usr/X11R6/lib  -lwx_gtk2_xrc-2.5 -lwx_gtk2_html-2.5 -lwx_gtk2_adv-2.5 -lwx_gtk2_core-2.5 -lwx_base_xml-2.5 -lwx_base_net-2.5 -lwx_base-2.5 


# List of full names for the object, dependency files
deps := $(objects:%.o=%.d)


#VPaths
vpath %.h $(prtsrcdir)
vpath %.cpp $(prtsrcdir)


# Rules
%.o : %.cpp
	../../../libtool --mode=compile $(CXX) -c -MMD $(CPPFLAGS) $(CXXFLAGS) $(DEFS) $< -o $@

# Compiles the main executable
prt_converter: $(objects)
	../../../libtool --mode=link $(CXX) $(LDFLAGS) $(LIBS) -o prt_converter $(objects)

# Removes the binaries
.PHONY: clean
clean:
	-rm -f *.o *.lo *.d ./prt_converter
	-rm -f -r .libs
	-rm -f $(objects:%=$(prtbindir)%)

-include *.d

