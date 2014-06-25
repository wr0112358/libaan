PROJECT_NAME=libaan
INSTALL_DIR=/opt/usr/include/$(PROJECT_NAME)

install:
	rm -rf $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)
	cp *.hh $(INSTALL_DIR)
