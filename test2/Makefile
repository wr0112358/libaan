all: unittest

LIBAAN_ROOT=..

CXX=g++
CXXFLAGS_DEBUG=-fsanitize=address -ggdb
CXXFLAGS=-Wall -std=c++1y -I$(LIBAAN_ROOT)
LDFLAGS=-lasan -Wl,-rpath ../libaan -L ../libaan -laan

algorithm_test.o: algorithm_test.cc $(LIBAAN_ROOT)/libaan/algorithm.hh
byte_test.o: byte_test.cc $(LIBAAN_ROOT)/libaan/byte.hh
crypto_test.o: crypto_test.cc
crypto_file_test.o: crypto_file_test.cc
debug_test.o: debug_test.cc
string_test.o: string_test.cc $(LIBAAN_ROOT)/libaan/string.hh
time_test.o: time_test.cc $(LIBAAN_ROOT)/libaan/time.hh
unittest.o: unittest.cc

unnittest: CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)
unittest: unittest.o algorithm_test.o byte_test.o crypto_test.o crypto_file_test.o debug_test.o string_test.o time_test.o
#	g++ -O2 -Wall -std=c++1y $^ -o $@ -lgtest -Wl,-rpath ../libaan -L ../libaan -laan
#	g++ $(CXXFLAGS) -std=c++1y $^ -o $@ -lgtest -Wl,-rpath ../libaan -L ../libaan -laan
	g++ -std=c++1y $^ -o $@ -lgtest $(LDFLAGS)

clean:
	rm -f *.o unittest tt3 tt2 tt

small: tt3 tt2 tt

tt3: tt3.o
	g++ $^ -o $@ $(LDFLAGS)

# fails
tt2:
	g++ -c tt2.cc -o tt2.o $(LDFLAGS)

tt: tt.o
	g++ $^ -o $@ $(LDFLAGS)
