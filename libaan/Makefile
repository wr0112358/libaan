WARN_MORE=-Wextra -Wpedantic -Wpointer-arith -Wcast-align -Wredundant-decls -Wdisabled-optimization -Wno-long-long -Wwrite-strings -Wcast-qual -pedantic-errors -fsanitize=address -ggdb
CXXFLAGS_TMP=-std=c++1y -Wall -Werror $(WARN_MORE)
CXXFLAGS=-fPIC -c -std=c++1y -Wall -Werror $(WARN_MORE)

SONAME=libaan.so
SONAME_VERS=$(SONAME).0.0.1

LDFLAGS=-shared -Wl,-soname,libaan.so.0.0.1 -lssl -lcrypto -lX11

all: $(SONAME_VERS)# tmp

crypto.o: crypto.cc crypto.hh
crypto_file.o: crypto_file.cc crypto_file.hh
debug.o: debug.cc debug.hh
file.o: file.cc file.hh
string.o: string.cc string.hh
terminal.o: terminal.cc terminal.hh
x11.o: x11.cc x11.hh

$(SONAME_VERS): crypto.o crypto_file.o debug.o file.o string.o terminal.o x11.o
	$(CXX) $(LDFLAGS) -o $@ $^
	ln -sf $(SONAME_VERS) $(SONAME)

# TODO: one .so per external dependency -> libaan_x11.so libaan_ssl.so

tmp: crypto_file_test.o test_x11_util.o
	$(CXX) $(CXXFLAGS_TMP) -o crypto_file_test \
		-L. crypto_file_test.cc -laan
	$(CXX) $(CXXFLAGS_TMP) -o test_x11_util \
		-L. test_x11_util.cc -laan

tmp_run: tmp

clean:
	rm -f $(SONAME) $(SONAME_VERS) *.o

install:
	cp *.hh /usr/include/libaan
	cp $(SONAME_VERS) /usr/lib64/
	ln -s /usr/lib64/$(SONAME_VERS) /usr/lib64/$(SONAME)
	cp ../share/libaan.pc /usr/lib64/pkgconfig

uninstall:
	rm -rf /usr/include/libaan/
	rm -f /usr/lib64/$(SONAME)
	rm -f /usr/lib64/$(SONAME_VERS)
	rm -f /usr/lib64/pkgconfig/libaan.pc
