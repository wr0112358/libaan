include Makefile.inc

dflt:
	@echo "No target specified."

install:
ifneq "$(USER)" "root"
	@echo "Need root"
	@exit 1
endif
	rm -rf /usr/include/$(PROJECT_NAME)/
	rm -f /usr/lib64/$(SONAME)
	rm -f /usr/lib64/$(SO_REALNAME)
	rm -f /usr/lib64/$(SO_LINKERNAME)
	mkdir -p /usr/include/$(PROJECT_NAME)
	cp $(PROJECT_NAME)/*.hh /usr/include/$(PROJECT_NAME)
	cp $(PROJECT_NAME)/$(SO_REALNAME) /usr/lib64/
	(cd /usr/lib64 && ln -s $(SO_REALNAME) $(SONAME))
	(cd /usr/lib64 && ln -s $(SONAME) $(SO_LINKERNAME))
	cp share/$(PROJECT_NAME).pc /usr/lib64/pkgconfig/

uninstall:
ifneq "$(USER)" "root"
	@echo "Need root"
	@exit 1
endif
	rm -f /usr/lib64/$(SONAME)
	rm -f /usr/lib64/$(SO_REALNAME)
	rm -f /usr/lib64/$(SO_LINKERNAME)
	rm -f /usr/lib64/pkgconfig/$(PROJECT_NAME).pc
	rm -rf /usr/include/$(PROJECT_NAME)/

build:
	make -C $(PROJECT_NAME)/ all
	make -C util/ all
	make -C test/ all
	make -C test/standalone all

clean:
	make -C $(PROJECT_NAME)/ clean
	make -C util/ clean
	make -C test/ clean
	make -C test/standalone clean
