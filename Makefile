# $Id$

#######################################################################
# Default compilation parameters. Normally don't edit these           #
#######################################################################

srcdir      ?= .

DEFINES     := -DHAVE_CONFIG_H
LDFLAGS     :=
INCLUDES    := -I. -I$(srcdir) -I$(srcdir)/engines
LIBS	    :=
OBJS	    :=

MODULES     :=
MODULE_DIRS :=

# Load the make rules generated by configure
include config.mk

CXXFLAGS:= -Wall $(CXXFLAGS)
CXXFLAGS+= -O -Wuninitialized
CXXFLAGS+= -Wno-long-long -Wno-multichar -Wno-unknown-pragmas
# Even more warnings...
CXXFLAGS+= -pedantic -Wpointer-arith -Wcast-qual -Wconversion
CXXFLAGS+= -Wshadow -Wimplicit -Wundef -Wnon-virtual-dtor
CXXFLAGS+= -Wno-reorder -Wwrite-strings

# Disable RTTI and exceptions, and enabled checking of pointers returned by "new"
CXXFLAGS+= -fno-rtti -fno-exceptions -fcheck-new

#######################################################################
# Misc stuff - you should never have to edit this                     #
#######################################################################

EXECUTABLE  := scummvm$(EXEEXT)

include $(srcdir)/Makefile.common

# check if configure has been run or has been changed since last run
config.mk: $(srcdir)/configure
	@echo "You need to run ./configure before you can run make"
	@echo "Either you haven't run it before or it has changed."
	@exit 1

install: all
	$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	$(INSTALL) -c -s -m 755 "$(srcdir)/scummvm$(EXEEXT)" "$(DESTDIR)$(BINDIR)/scummvm$(EXEEXT)"
	$(INSTALL) -d "$(DESTDIR)$(MANDIR)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/scummvm.6" "$(DESTDIR)$(MANDIR)/man6/scummvm.6"
	$(INSTALL) -d "$(DESTDIR)$(PREFIX)/share/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/scummvm.xpm" "$(DESTDIR)$(PREFIX)/share/pixmaps/scummvm.xpm"
	$(INSTALL) -d "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"
	$(INSTALL) -c -m 644 "$(srcdir)/AUTHORS" "$(srcdir)/COPYING" "$(srcdir)/NEWS" "$(srcdir)/README" "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/scummvm$(EXEEXT)"
	rm -f "$(DESTDIR)$(MANDIR)/man6/scummvm.6"
	rm -f "$(DESTDIR)$(PREFIX)/share/pixmaps/scummvm.xpm"
	rm -rf "$(DESTDIR)$(PREFIX)/share/doc/scummvm/"

scummvmico.o: icons/scummvm.ico
	windres dists/scummvm.rc scummvmico.o

dist:
	$(RM) $(ZIPFILE)
	$(ZIP) $(ZIPFILE) $(DISTFILES)

deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary


# Special target to create a application wrapper for Mac OS X
bundle_name = ScummVM.app
bundle: scummvm-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(srcdir)/dists/macosx/Info.plist $(bundle_name)/Contents/
	cp $(srcdir)/icons/scummvm.icns $(bundle_name)/Contents/Resources/
	cp $(srcdir)/gui/themes/modern.ini $(bundle_name)/Contents/Resources/
	cp $(srcdir)/gui/themes/modern.zip $(bundle_name)/Contents/Resources/
	cp scummvm-static $(bundle_name)/Contents/MacOS/scummvm
	$(srcdir)/tools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	strip $(bundle_name)/Contents/MacOS/scummvm

# location of additional libs for OS X usually /sw/ for fink or
# /opt/local/ for darwinports
OSXOPT=/sw
# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
scummvm-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o scummvm-static $(OBJS) \
		`sdl-config --static-libs` \
		-framework CoreMIDI \
		$(OSXOPT)/lib/libmad.a \
		$(OSXOPT)/lib/libvorbisfile.a \
		$(OSXOPT)/lib/libvorbis.a \
		$(OSXOPT)/lib/libogg.a \
		$(OSXOPT)/lib/libmpeg2.a \
		$(OSXOPT)/lib/libFLAC.a \
		-lSystemStubs \
		-lz

# Target for building the PDF version of the README
doc/readme.pdf: doc/readme.tex doc/*.tex
	cd doc && pdflatex readme.tex 
	cd doc && pdflatex readme.tex 

# Special target to create a snapshot disk image for Mac OS X
osxsnap: bundle doc/readme.pdf
	mkdir ScummVM-snapshot
	cp COPYING ./ScummVM-snapshot/License
	cp NEWS ./ScummVM-snapshot/News
	cp AUTHORS ./ScummVM-snapshot/Authors
	/Developer/Tools/SetFile -t ttro -c ttxt ./ScummVM-snapshot/*
	cp doc/readme.pdf ./ScummVM-snapshot/ScummVM\ ReadMe
	/Developer/Tools/SetFile -t 'PDF ' -c prvw ./ScummVM-snapshot/ScummVM\ ReadMe
	/Developer/Tools/CpMac -r $(bundle_name) ./ScummVM-snapshot/
	cp dists/macosx/DS_Store ./ScummVM-snapshot/.DS_Store
	cp dists/macosx/background.jpg ./ScummVM-snapshot/background.jpg
	/Developer/Tools/SetFile -a V ./ScummVM-snapshot/.DS_Store
	/Developer/Tools/SetFile -a V ./ScummVM-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9  \
					-srcfolder ScummVM-snapshot \
					-volname "ScummVM snapshot" \
					ScummVM-snapshot.dmg
	rm -rf ScummVM-snapshot

# Special target to create a win32 snapshot binary
win32dist: scummvm$(EXEEXT)
	mkdir -p $(WIN32PATH)
	strip scummvm.exe -o $(WIN32PATH)/scummvm$(EXEEXT)
	cp gui/themes/modern.ini $(WIN32PATH)
	cp gui/themes/modern.zip $(WIN32PATH)
	cp AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp COPYING $(WIN32PATH)/COPYING.txt
	cp NEWS $(WIN32PATH)/NEWS.txt
	cp README $(WIN32PATH)/README.txt
	cp /usr/local/README-SDL.txt $(WIN32PATH)
	cp /usr/local/bin/SDL.dll $(WIN32PATH)
	u2d $(WIN32PATH)/*.txt

# Special target to create an AmigaOS snapshot installation
aos4dist: scummvm
	mkdir -p $(AOS4PATH)
	strip -R.comment $< -o $(AOS4PATH)/$<
	cp icons/scummvm.info $(AOS4PATH)/$<.info
	cp gui/themes/modern.ini $(AOS4PATH)
	cp gui/themes/modern.zip $(AOS4PATH)
	cp AUTHORS $(AOS4PATH)/AUTHORS.txt
	cp COPYING $(AOS4PATH)/COPYING.txt
	cp NEWS $(AOS4PATH)/NEWS.txt
	cp README $(AOS4PATH)/README.txt
	cp /sdk/local/documentation/SDL-1.2.9/README-SDL.txt $(AOS4PATH)

.PHONY: deb bundle osxsnap win32dist dist install uninstall
