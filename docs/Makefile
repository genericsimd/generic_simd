# Make file to build the files


default: html


html: intrinsics.doxyfile ../include/power_vsx4.h mainpage.txt
	doxygen $<

publish: html
	cp -R html/* /gsa/yktgsa/projects/g/generic_simd/docs/

clean:
	@rm -fR html
