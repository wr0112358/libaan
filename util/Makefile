include ../Makefile.inc

LDFLAGS=-lssl -lcrypto -lX11
#LDFLAGS=$(pkg-config --libs libaan)

all: convert_crypto_file_0010to0020

CXXFLAGS+=-I$(PROJECT_ROOT)
LDFLAGS=-lasan -Wl,-rpath ../libaan -L ../libaan -laan

all: convert_crypto_file_0010to0020

convert_crypto_file_0010to0020.o: convert_crypto_file_0010to0020.cc

convert_crypto_file_0010to0020: convert_crypto_file_0010to0020.o

%: %.o
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o convert_crypto_file_0010to0020
