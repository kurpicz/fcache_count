################################################################################
# test-fcache-profile/Makefile
#
# Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

CXXFLAGS = -std=c++14 -I.. -pthread
LDFLAGS =
LIBS = -ldl

FC_OBJS = ../fcache_count.o ../fcache_disable.o

all: test

../fcache_count.o: ../fcache_count.cpp
	$(CXX) $(CXXFLAGS) -DFCACHE_DISABLED=1 -c -o $(@D)/disabled_$(@F) $<
	$(CXX) $(CXXFLAGS) -c -o $(@D)/enabled_$(@F) $<

../fcache_disable.o: ../fcache_disable.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: test.cpp $(FC_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) test.cpp\
		-o $@_disabled ../disabled_fcache_count.o ../fcache_disable.o $(LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) test.cpp\
		-o $@_enabled ../enabled_fcache_count.o $(LIBS)

clean:
	rm -f ../*.o test_disabled test_enabled fcache_profile

################################################################################
