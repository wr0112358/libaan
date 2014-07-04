include ../Makefile.in

test: test_x11_util crypto_util_test crypto_util_test_camellia test_terminal test_string_util
#crypto_util_test_lion 

test_x11_util: test_x11_util.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@ -lX11 -o test_x11_util

test_terminal: test_terminal.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@

crypto_util_test: crypto_util_test.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@ -lssl -lcrypto
crypto_util_test_camellia: crypto_util_test_camellia.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@ -lssl -lcrypto
crypto_util_test_lion: crypto_util_test_lion.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@ -lssl -lcrypto

test_string_util: test_string_util.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@

clean:
	rm -f test_terminal test_x11_util crypto_util_test crypto_util_test_camellia test_string_util
