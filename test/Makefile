include ../Makefile.inc

LDFLAGS=-lssl -lcrypto -lX11
#LDFLAGS=$(pkg-config --libs libaan)

all: build_gtest unittest

CXXFLAGS+=-I$(PROJECT_ROOT)
#LDFLAGS=-lasan -Wl,-rpath ../libaan -L ../libaan -laan
LDFLAGS=-Wl,-rpath ../libaan -L ../libaan -laan

algorithm_test.o: algorithm_test.cc $(PROJECT_ROOT)/libaan/algorithm.hh
bit_vector_test.o: bit_vector_test.cc
byte_test.o: byte_test.cc $(PROJECT_ROOT)/libaan/byte.hh
crypto_test.o: crypto_test.cc
crypto_file_test.o: crypto_file_test.cc
debug_test.o: debug_test.cc
string_test.o: string_test.cc $(PROJECT_ROOT)/libaan/string.hh
time_test.o: time_test.cc $(PROJECT_ROOT)/libaan/time.hh
unittest.o: unittest.cc

ALL_OBJS = unittest.o algorithm_test.o bit_vector_test.o byte_test.o crypto_test.o crypto_file_test.o debug_test.o string_test.o time_test.o


unittest: LDFLAGS+=.build_gtest/gtest-1.7.0/lib/.libs/libgtest.a -pthread
unittest: CXXFLAGS+=-isystem .build_gtest/gtest-1.7.0/include/ -DWORDSFILE=\"$(WORDSFILE)\" -Wsign-conversion
unittest: $(ALL_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o unittest tt3 tt2 tt
	./gtest.sh clean

.PHONY: build_gtest
build_gtest:
	./gtest.sh clean
	GTEST_CXXFLAGS="$(CXXFLAGS) -Wno-error=missing-field-initializers -Wno-missing-field-initializers" GTEST_CXX=$(CXX) ./gtest.sh build
