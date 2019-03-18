# Copyright (c) 2003-2019 Doug Rogers under the Zero Clause BSD License.
# You are free to do whatever you want with this software. See LICENSE.txt.

TARGET = example_unit_test
CC_TARGET = cc_example_unit_test

OBJS = \
	$(TARGET).o \
	example_test.o \
	example_with_init_test.o \
	cut.o

CC_OBJS = \
	cc_example_unit_test.o \
	ccut.o \
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

%.o : %.cc
	$(CXX) -o $@ $(CXXFLAGS) -c $<

%.o : %.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<


all: $(TARGET) $(CC_TARGET) complex_test

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(PLATFORM_LIBS)

$(CC_TARGET): $(CC_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) $(PLATFORM_LIBS)

complex_test: complex_test.o cut.o
	$(CC) $(CFLAGS) -o $@ $^

test: all
	./$(TARGET)
	./$(CC_TARGET)


.PHONY: clean
clean:
	rm -f *~ *.o *.d $(TARGET) $(CC_TARGET) complex_test Makefile.depend core
	rm -rf html

ifneq ($(MAKECMDGOALS),clean)

SED_PATTERN = 's/\([^ ].*\)\.o[ :]*/\1.o \1.d : /g'

%.d: %.c
	$(CC) -M $(DEFINES) $(INCLUDES) $< | sed $(SED_PATTERN) > $@

%.d: %.cc
	$(CXX) -M $(DEFINES) $(INCLUDES) $< | sed $(SED_PATTERN) > $@

%.d: %.cpp
	$(CXX) -M $(DEFINES) $(INCLUDES) $< | sed $(SED_PATTERN) > $@

.PHONY: doc
doc:
	doxygen doxygen.cfg

# make will build the .d file and then include it.

Makefile.depend: $(OBJS:.o=.d) $(CC_OBJS:.o=.d)
	@rm -f Makefile.depend
	@for file in $^; do echo "include $$file" >> $@; done
	@echo "Updating $@."

include Makefile.depend

endif
