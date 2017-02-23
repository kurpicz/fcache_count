CXXFLAGS = -std=c++17 -I.. -pthread
LDFLAGS =
LIBS = -ldl
OBJS = test.o ../fcache_count.o

all: test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *.o test
