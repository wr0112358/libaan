include Makefile.in

install:
	rm -rf $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)
	cp *.hh $(INSTALL_DIR)
