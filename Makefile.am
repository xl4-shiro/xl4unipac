#
# Excelfore Universal IPC and Configuration code generator
# Copyright (C) 2020 Excelfore Corporation (https://excelfore.com)
#
# This file is part of Excelfore-unipac.
#
# Excelfore-unipac is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Excelfore-unipac is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Excelfore-unipac.  If not, see
# <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
#
PACKAGENAME=xl4unipac
VERSION=1.0.0
DESCRIPTION=Excelfore Universal IPC and Configuration tools

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall -DUB_LOGCAT=2

SAMPLE_PREFIX=sample

if BUILD_IN_STPL
AM_CFLAGS += -I$(srcdir)/..
AM_LDFLAGS = -L../xl4unibase/.libs -L../xl4combase/.libs
DOC_DIR = $(abs_srcdir)/../xl4unibase/doc
else
AM_CFLAGS += -I$(includedir)/xl4tsn
AM_LDFLAGS =
DOC_DIR = $(datadir)/xl4unibase
endif

if NO_THREAD
AM_CFLAGS += -DCB_THREAD_NON_POSIX_H=\"$(SAMPLE_PREFIX)_non_thread.h\"
NTOPTION = --ntfile
else
AM_LDFLAGS += -lpthread
NTOPTION =
endif

pkgdatadir=${datarootdir}/$(PACKAGENAME)
bin_SCRIPTS = up_genconf.py

install-data-local: temp_configs.c temp_configs.h temp_ipcconfigs.c temp_ipcconfigs.h \
temp_ipcclient.c temp_non_thread.h temp_ipcserver.c
	install -d $(DESTDIR)$(pkgdatadir)
	$(INSTALL) $^ $(DESTDIR)$(pkgdatadir)/

GENERATED_CODES = sample_configs.c sample_configs.h sample_ipcconfigs.c sample_ipcconfigs.h \
sample_configs_test.c sample_ipcclient.c \
sample_defaults.conf sample_non_thread.h

$(GENERATED_CODES): \
temp_configs.c temp_configs.h sample_defaults.cfg up_genconf.py \
temp_ipcconfigs.c temp_ipcconfigs.h temp_ipcclient.c
	cd $(srcdir);\
	./up_genconf.py -p $(SAMPLE_PREFIX) --hfile --cfile --tfile -i sample_defaults.cfg \
	-m sample_defaults.conf --ipcfile --clfile $(NTOPTION)
	sed -r "s/23/32/g;s/^#(\S)/\1/g" $(srcdir)/sample_defaults.conf > tsample_defaults.conf
	mv -f tsample_defaults.conf sample_defaults.conf

check_PROGRAMS = sample_configs_test sample_configs_manual_test sample_ipcconfigs_manual_test \
	sample_ipcclient
TESTS = sample_configs_test sample_configs_manual_test
if ! NO_THREAD
TESTS += sample_ipcconfigs_manual_test
endif

sample_configs_test_SOURCES = sample_configs.c sample_configs.h sample_configs_test.c
sample_configs_test_CFLAGS = $(AM_CFLAGS)
sample_configs_test_LDADD = -lx4unibase -lx4combase

sample_configs_manual_test_SOURCES = sample_configs.c sample_configs.h \
sample_configs_manual_test.c
sample_configs_manual_test_CFLAGS = $(AM_CFLAGS)
sample_configs_manual_test_LDADD = -lx4unibase -lx4combase

sample_ipcconfigs_manual_test_SOURCES = sample_ipcconfigs.c sample_ipcconfigs.h \
sample_configs.c sample_configs.h sample_ipcconfigs_manual_test.c
sample_ipcconfigs_manual_test_CFLAGS = $(AM_CFLAGS)
sample_ipcconfigs_manual_test_LDADD = -lx4unibase -lx4combase

sample_ipcclient_SOURCES = sample_ipcconfigs.c sample_ipcconfigs.h \
sample_configs.c sample_configs.h sample_ipcclient.c
sample_ipcclient_CFLAGS = $(AM_CFLAGS)
sample_ipcclient_LDADD = -lx4unibase -lx4combase

doc: $(PACKAGENAME)_usersguide.pdf
TARGET_PDF=$(PACKAGENAME)_usersguide.pdf
RESOURCE_FILES=excelfore_logo.png xl4_usersguide_template.org
$(RESOURCE_FILES):
	@tddir=$(DOC_DIR); \
	if [ ! -f $${tddir}/$@ ]; then \
		tddir=$${tddir/\/usr/\/usr\/local}; \
	fi; \
	if [ ! -f $${tddir}/$@ ]; then \
		echo "$${tddir}/excelfore_logo.png doesn't exist"; \
		exit 1;\
	fi; \
	cp $${tddir}/$@ .
$(PACKAGENAME)_usersguide.org: $(RESOURCE_FILES) README.org
	cat xl4_usersguide_template.org $(srcdir)/README.org > $(PACKAGENAME)_usersguide.org
	sed -i "s:\\\\textcolor{cyan}{Confidential}::" $(PACKAGENAME)_usersguide.org
	sed -i "s/XL4-DOCUMENT-TITLE/$(PACKAGENAME) Usersguide/" $(PACKAGENAME)_usersguide.org
	sed -i "s/XL4-DOCUMENT-FOOTER-TITLE/$(PACKAGENAME) Usersguide/" \
	$(PACKAGENAME)_usersguide.org
	DOCDATE=`git show -s --format=%ci HEAD | sed -r "s/(\S*).*/\1/"`; \
	sed -i "s/XL4-DOCUMENT-DATE/$${DOCDATE}/g" $(PACKAGENAME)_usersguide.org

$(TARGET_PDF): $(PACKAGENAME)_usersguide.org
	emacs xl4unipac_usersguide.org --batch -f org-latex-export-to-pdf --kill

install-pdf-local: $(TARGET_PDF)
	install -d $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $^ $(DESTDIR)$(docdir)/
uninstall-pdf: $(TARGET_PDF)
	rm -rf $(DESTDIR)$(docdir)/*.pdf
	if [ -d $(DESTDIR)$(docdir) -a -z "`ls -A $(DESTDIR)$(docdir)`" ]; then \
		rmdir $(DESTDIR)$(docdir); \
	fi

clean-generic:
	rm -f *.pdf *.tex *.org
	cd $(srcdir); rm -f $(GENERATED_CODES)

VALGRIND=valgrind
check-mem: $(TESTS)
	for test in $(TESTS) ; do \
	./libtool --mode=execute $(VALGRIND) \
		--tool=memcheck --leak-check=full --leak-resolution=low \
		--track-origins=yes --track-fds=yes --time-stamp=yes \
		--error-exitcode=1 --show-leak-kinds=all ./$${test}; \
	done

deb:
	@cd $(srcdir); make -f debian.make
