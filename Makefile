include Makefile.inc

dflt:
	@echo "No target specified. Try make clean build && sudo make install"

install:
ifneq "$(USER)" "root"
	@echo "Need root"
	@exit 1
endif
	rm -rf /usr/include/$(PROJECT_NAME)/
	rm -f $(INSTALL_PATH)/$(SONAME)
	rm -f $(INSTALL_PATH)/$(SO_REALNAME)
	rm -f $(INSTALL_PATH)/$(SO_LINKERNAME)
	mkdir -p /usr/include/$(PROJECT_NAME)
	cp $(PROJECT_NAME)/*.hh /usr/include/$(PROJECT_NAME)
	cp $(PROJECT_NAME)/$(SO_REALNAME) $(INSTALL_PATH)/
	(cd $(INSTALL_PATH) && ln -s $(SO_REALNAME) $(SONAME))
	(cd $(INSTALL_PATH) && ln -s $(SONAME) $(SO_LINKERNAME))
	cp share/$(PROJECT_NAME).pc $(INSTALL_PATH)/pkgconfig/

uninstall:
ifneq "$(USER)" "root"
	@echo "Need root"
	@exit 1
endif
	rm -f $(INSTALL_PATH)/$(SONAME)
	rm -f $(INSTALL_PATH)/$(SO_REALNAME)
	rm -f $(INSTALL_PATH)/$(SO_LINKERNAME)
	rm -f $(INSTALL_PATH)/pkgconfig/$(PROJECT_NAME).pc
	rm -rf /usr/include/$(PROJECT_NAME)/

build:
	$(MAKE) -C $(PROJECT_NAME)/ all
	$(MAKE) -C util/ all
	$(MAKE) -C test/ all
	$(MAKE) -C test/standalone all

clean:
	$(MAKE) -C $(PROJECT_NAME)/ clean
	$(MAKE) -C util/ clean
	$(MAKE) -C test/ clean
	$(MAKE) -C test/standalone clean
