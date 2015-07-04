include ../../Makefile.inc

LDFLAGS=-lssl -lcrypto -lX11
#LDFLAGS=$(pkg-config --libs libaan)

all: tt tt3 test_terminal tmp snippets

CXXFLAGS+=-I$(PROJECT_ROOT)
LDFLAGS=-lasan -Wl,-rpath ../../libaan -L ../../libaan -laan

clean:
	rm -f *.o tt3 tt2 tt test_terminal crypto_file_test test_x11_util snippets

%:%.o
	$(CXX) $^ -o $@ $(LDFLAGS)

tt3: tt3.o
tt: tt.o
test_terminal: test_terminal.o

# fails
tt2:
	$(CXX) -c tt2.cc -o tt2.o $(LDFLAGS)

tmp: crypto_file_test.o test_x11_util.o
	$(CXX) $(CXXFLAGS) -o crypto_file_test \
		crypto_file_test.cc $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o test_x11_util \
		test_x11_util.cc $(LDFLAGS)

snippets: snippets.o
	$(CXX) $^ -o $@ $(LDFLAGS)
