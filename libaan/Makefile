include ../Makefile.inc

CXXFLAGS+=-fPIC

LDFLAGS+=-shared -Wl,-soname,$(SONAME) -lssl -lcrypto -lX11

all: $(SO_REALNAME)# tmp

crypto.o: crypto.cc crypto.hh
crypto_file.o: crypto_file.cc crypto_file.hh
debug.o: debug.cc debug.hh
file.o: file.cc file.hh
string.o: string.cc string.hh
terminal.o: terminal.cc terminal.hh
x11.o: x11.cc x11.hh

ALL_OBJS=crypto.o crypto_file.o debug.o file.o string.o terminal.o x11.o

$(SO_REALNAME): $(ALL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^
	ln -sf $(SO_REALNAME) $(SONAME)
	ln -sf $(SONAME) $(SO_LINKERNAME) 

clean:
	rm -f $(ALL_OBJS) $(SO_REALNAME) $(SONAME) $(SO_LINKERNAME)


# TODO: one .so per external dependency? -> libaan_x11.so libaan_ssl.so

tt.o: tt.cc
tt: tt.o
	$(CXX) -o $@ $^
