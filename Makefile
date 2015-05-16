include Makefile.in
all:
	@echo "No target specified."

install:
	rm -rf $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)
	cp *.hh $(INSTALL_DIR)
