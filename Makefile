#converts cr/lf to just lf
DOS2UNIX = dos2unix

all: default

default: dist

dist: FreeImage
	cp *.a Dist
	cp *.so Dist
	cp Source/FreeImage.h Dist

FreeImage:
	@if [ ! -f .nodos2unix ]; then \
	$(DOS2UNIX) Source/*.[ch]* Source/FreeImage/*.[ch]* \
	Source/LibJPEG/*.[ch]* Source/LibMNG/*.[ch]* Source/LibPNG/*.[ch]* \
	Source/LibTIFF/*.[ch]* Source/ZLib/*.[ch]* \
	Examples/Linux/*.[ch]* ; \
	touch .nodos2unix; \
	fi
	make -f Makefile.linux all

refix:
	rm .nodos2unix

install:
	make -f Makefile.linux install

clean:
	rm -f Dist/* u2dtmp* .nodos2unix
	make -f Makefile.linux clean
	make -C Examples clean
