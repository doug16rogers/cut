# Copyright (c) 2010 Doug Rogers under the terms of the MIT License.
# See http://www.opensource.org/licenses/mit-license.html..
# $Id$

TARGET = example_unit_test

OBJS = \
	$(TARGET).o \
	example_simple.o \
	example_complex.o \
	cut.o

DEFINES  = $(PLATFORM_DEFINES)
INCLUDES = $(PLATFORM_INCLUDES)

CC  = gcc
CXX = g++

CFLAGS   = -g -Wall -Werror $(PLATFORM_CFLAGS) $(DEFINES) $(INCLUDES)

#
# Implicit rules to compile C and C++ code.
#
%.o : %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.o : %.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(PLATFORM_LIBS)

test: all
	./$(TARGET)


clean:
	rm -f *~ *.o *.d $(TARGET) Makefile.depend core
	rm -rf html

ifneq ($(MAKECMDGOALS),clean)

SED_PATTERN = 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g'

%.d: %.c
	$(CC) -M $(DEFINES) $(INCLUDES) $< | sed $(SED_PATTERN) > $@

%.d: %.cpp
	$(CXX) -M $(DEFINES) $(INCLUDES) $< | sed $(SED_PATTERN) > $@

.PHONY: doc

doc:
	doxygen doxygen.cfg

# make will build the .d file and then include it.

Makefile.depend: $(OBJS:.o=.d)
	@rm -f Makefile.depend
	@for file in $^; do echo "include $$file" >> $@; done
	@echo "Updating $@."

include Makefile.depend

endif

