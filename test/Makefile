include ../Makefile.in

test: test_x11_util crypto_file_test crypto_util_test crypto_util_test_camellia test_terminal test_string_util test_string_suffix_search test_string_suffix_search_dc3 crypto_hash_test convert_crypto_file_0010to0020
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

crypto_hash_test: crypto_hash_test.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@ -lssl -lcrypto

test_string_util: test_string_util.cc
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $? -o $@

crypto_file_test: crypto_file_test.cc
	$(CXX) $(CXX_FLAGS) -lssl -lcrypto $^ -o $@

convert_crypto_file_0010to0020: convert_crypto_file_0010to0020.cc
	$(CXX) $(CXX_FLAGS) -lssl -lcrypto $^ -o $@

test_string_suffix_search: test_string_suffix_search.cc ../string_suffix_search.cc ../string_suffix_search_dc3_internal.cc
	$(CXX) $(CXX_FLAGS)  $^ -o $@

test_string_suffix_search_dc3: test_string_suffix_search_dc3.cc ../string_suffix_search.cc ../string_suffix_search_dc3_internal.cc
	$(CXX) $(CXX_FLAGS) -D_DEBUG_TIMINGS_ $^ -o $@

clean:
	rm -f test_terminal test_x11_util crypto_util_test crypto_util_test_camellia test_string_util crypto_file_test test_string_suffix_search_dc3 test_string_suffix_search
