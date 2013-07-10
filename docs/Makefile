# Make file to build the files

GSIMD_DOC_ROOT=/gsa/yktgsa/projects/g/generic_simd/docs/
default: html


html: intrinsics.doxyfile ../include/power_vsx4.h mainpage.txt
	doxygen $<

publish: html
	cp -R html/* ${GSIMD_DOC_ROOT}
	chmod -R g+wxr ${GSIMD_DOC_ROOT}
	chmod -R a+xr ${GSIMD_DOC_ROOT}

clean:
	@rm -fR html
