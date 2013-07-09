# Make file to build the files


default: html


html: intrinsics.doxyfile ../intrinsics/power_vsx4.h mainpage.txt
	doxygen $<

clean:
	@rm -fR html
