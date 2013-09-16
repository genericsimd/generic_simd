# Make file to build the files

PUBLISH_ROOT=gsimd_html
GH_PAGES_ROOT=gh-pages.github
default: html

.PHONY: html

html: intrinsics.doxyfile ../include/power_vsx4.h mainpage.txt developer_guide.txt history.txt issues.txt
	doxygen $<

gitpub: html
	cp -fR html/* ${GH_PAGES_ROOT}

copy: html
	cp -R html/* ${PUBLISH_ROOT}
	chmod -R g+wxr ${PUBLISH_ROOT}/*
	chmod -R a+xr ${PUBLISH_ROOT}/*

clean:
	@rm -fR html
