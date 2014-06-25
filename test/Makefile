include ../Makefile.in

PROJECT_NAME=wr011_lib
INSTALL_DIR=/opt/usr/install/$(PROJECT_NAME)

test: test_terminal test_x11_util

test_x11_util: test_x11_util.cc
	$(CXX) $(CXX_FLAGS) -Wall test_x11_util.cc -lX11 -o test_x11_util

test_terminal: test_terminal.cc
	$(CXX) $(CXX_FLAGS) test_terminal.cc -o test_terminal

clean:
	rm -f test_terminal test_x11_util
