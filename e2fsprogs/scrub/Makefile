#
# Makefile for e2scrub
#

srcdir = .
top_srcdir = ..

top_builddir = ..
my_dir = scrub
INSTALL = /usr/bin/install -c
MKDIR_P = /bin/mkdir -p

# Beginning of file MCONFIG

all::

all-static::

check::

fullcheck::

SHELL = /bin/sh

COMPRESS_EXT = gz bz2 bz Z

prefix = /usr
root_prefix = 
exec_prefix = ${prefix}
root_bindir = ${root_prefix}/bin
root_sbindir = ${root_prefix}/sbin
root_libdir = ${root_prefix}/lib
datarootdir = ${prefix}/share
bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libdir = ${exec_prefix}/lib
datadir= ${datarootdir}
localedir = $(datadir)/locale
root_sysconfdir= ${root_prefix}/etc
includedir = ${prefix}/include
mandir = ${datarootdir}/man
man1dir = $(mandir)/man1
man3dir = $(mandir)/man3
man5dir = $(mandir)/man5
man8dir = $(mandir)/man8
infodir = ${datarootdir}/info
datadir = ${datarootdir}
pkgconfigdir = $(libdir)/pkgconfig
pkglibdir = $(libdir)/e2fsprogs

HAVE_UDEV = no
UDEV_RULES_DIR = 
HAVE_CROND = yes
CROND_DIR = /etc/cron.d
HAVE_SYSTEMD = no
SYSTEMD_SYSTEM_UNIT_DIR = 



 V =
 ifeq ($(strip $(V)),)
 #  E = @echo
 #  ES = echo
 #  Q = @
    E = @echo
    ES = echo
    Q = @
 else
    E = @\#
    ES = \#
    Q =
 endif

# E = @echo
# ES = echo
# Q = @

 CHECK=sparse
 CHECK_OPTS=-Wsparse-all -Wno-transparent-union -Wno-return-void -Wno-undef -Wno-non-pointer-null
 CPPCHECK=cppcheck
 CPPCHECK_OPTS=--force --enable=all --quiet
 ifeq ("$(C)", "2")
   CHECK_CMD=$(CHECK) $(CHECK_OPTS) -Wbitwise -D__CHECK_ENDIAN__
   CPPCHECK_CMD=$(CPPCHECK) $(CPPCHECK_OPTS)
 else
   ifeq ("$(C)", "1")
     CHECK_CMD=$(CHECK) $(CHECK_OPTS)
     CPPCHECK_CMD=$(CPPCHECK) $(CPPCHECK_OPTS)
    else
     CHECK_CMD=@true
     CPPCHECK_CMD=@true
   endif
 endif

# CHECK_CMD=true
# CPPCHECK_CMD=true

SANITIZER_CFLAGS =    
SANITIZER_LDFLAGS =    

CC = gcc
BUILD_CC = gcc
PTHREAD_CFLAGS = -pthread
PTHREAD_LIBS = 
CFLAGS = -g -O2
CFLAGS_SHLIB = -g -O2
CFLAGS_STLIB = -g -O2
CPPFLAGS = -I. -I$(top_builddir)/lib -I$(top_srcdir)/lib
ALL_CFLAGS = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
ALL_CFLAGS_SHLIB = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS_SHLIB) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
ALL_CFLAGS_STLIB = $(CPPFLAGS) $(SANITIZER_CFLAGS) $(CFLAGS_STLIB) $(PTHREAD_CFLAGS) $(CFLAGS_WARN) -DHAVE_CONFIG_H $(LOCAL_CFLAGS)
LDFLAGS = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS) 
LDFLAGS_SHLIB = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS) 
ALL_LDFLAGS = $(LDFLAGS) 
LDFLAGS_STATIC = $(SANITIZER_LDFLAGS) $(PTHREAD_CFLAGS)  -static
BUILD_CFLAGS = $(SANITIZER_CFLAGS) -g -O2  -I. -I$(top_builddir)/lib -I$(top_srcdir)/lib -DHAVE_CONFIG_H
BUILD_LDFLAGS = $(SANITIZER_LDFLAGS) 
RDYNAMIC = -rdynamic
LINK_BUILD_FLAGS = 
LINK_INSTALL_FLAGS = -f
RM = /bin/rm
LN = /bin/ln
LN_S = ln -s
MV = /bin/mv
CP = /bin/cp
CHMOD = /bin/chmod
AR = ar
AWK = mawk
SED = /bin/sed
PERL = /usr/bin/perl
RANLIB = ranlib
STRIP = strip
LD = $(PURE) gcc
ARUPD = $(AR) r
ARGEN = $(AR) rc
LDCONFIG = /sbin/ldconfig
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}

#
# Library definitions
#
LIB = $(top_builddir)/lib
LIBSS = $(LIB)/libss.a  -ldl
LIBCOM_ERR = $(LIB)/libcom_err.a  -lpthread
LIBE2P = $(LIB)/libe2p.a
LIBEXT2FS = $(LIB)/libext2fs.a
LIBUUID = $(LIB)/libuuid.a 
LIBMAGIC = -ldl
LIBFUSE = 
LIBSUPPORT = $(LIBINTL) $(LIB)/libsupport.a
LIBBLKID = $(LIB)/libblkid.a  $(LIBUUID)
LIBINTL = 
SYSLIBS =  
DEPLIBSS = $(LIB)/libss.a
DEPLIBCOM_ERR = $(LIB)/libcom_err.a
DEPLIBUUID = $(LIB)/libuuid.a
DEPLIBSUPPORT = $(LIB)/libsupport.a
DEPLIBBLKID = $(LIB)/libblkid.a  $(DEPLIBUUID)
TESTENV = LD_LIBRARY_PATH="$(LIB):$${LD_LIBRARY_PATH}" DYLD_LIBRARY_PATH="$(LIB):$${DYLD_LIBRARY_PATH}"

STATIC_LIBSS = $(LIB)/libss.a -ldl
STATIC_LIBCOM_ERR = $(LIB)/libcom_err.a -lpthread
STATIC_LIBE2P = $(LIB)/libe2p.a
STATIC_LIBEXT2FS = $(LIB)/libext2fs.a
STATIC_LIBUUID = $(LIB)/libuuid.a 
STATIC_LIBSUPPORT = $(LIBINTL) $(LIBSUPPORT)
STATIC_LIBBLKID = $(LIB)/libblkid.a $(STATIC_LIBUUID)
DEPSTATIC_LIBSS = $(LIB)/libss.a
DEPSTATIC_LIBCOM_ERR = $(LIB)/libcom_err.a
DEPSTATIC_LIBUUID = $(LIB)/libuuid.a
DEPSTATIC_LIBSUPPORT = $(DEPLIBSUPPORT)
DEPSTATIC_LIBBLKID = $(LIB)/libblkid.a $(DEPSTATIC_LIBUUID)

PROFILED_LIBSS = $(LIB)/libss.a -ldl
PROFILED_LIBCOM_ERR = $(LIB)/libcom_err.a -lpthread
PROFILED_LIBE2P = $(LIB)/libe2p.a
PROFILED_LIBEXT2FS = $(LIB)/libext2fs.a
PROFILED_LIBUUID = $(LIB)/libuuid.a 
PROFILED_LIBSUPPORT = $(LIBINTL) $(LIB)/libsupport.a
PROFILED_LIBBLKID = $(LIB)/libblkid.a $(PROFILED_LIBUUID)
DEPPROFILED_LIBSS = $(LIB)/libss.a
DEPPROFILED_LIBCOM_ERR = $(LIB)/libcom_err.a
DEPPROFILED_LIBUUID = $(LIB)/libuuid.a
DEPPROFILED_LIBSUPPORT = $(PROFILED_LIBSUPPORT)
DEPPROFILED_LIBBLKID = $(LIB)/libblkid.a $(DEPPROFILED_LIBUUID)

#
# A fast substitution command for fixing up man pages, shell scripts, etc.
#
SUBST_CONF=$(top_builddir)/util/subst.conf
SUBSTITUTE= $(top_builddir)/util/subst -f $(SUBST_CONF)
SUBSTITUTE_UPTIME= $(top_builddir)/util/subst -t -f $(SUBST_CONF)
DEP_SUBSTITUTE= $(top_builddir)/util/subst $(SUBST_CONF)

$(top_builddir)/util/subst:
	cd $(top_builddir)/util ; $(MAKE) subst

#
# Script for generating utf8data.h
#
MKUTF8DATA=$(top_builddir)/util/mkutf8data

$(top_builddir)/util/mkutf8data:
	 $(MAKE) -C $(top_builddir)/util mkutf8data

#
# Script for installing symlinks (for shared libraries)
#
$(top_builddir)/util/install-symlink: $(top_srcdir)/util/install-symlink.in \
			      $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=util/install-symlink ./config.status
	chmod +x $(top_builddir)/util/install-symlink

$(top_builddir)/util/symlinks:
	cd $(top_builddir)/util ; $(MAKE) symlinks

INSTALL_SYMLINK = /bin/sh $(top_builddir)/util/install-symlink \
			 \
			--symlinks=$(top_builddir)/util/symlinks
DEP_INSTALL_SYMLINK = $(top_builddir)/util/install-symlink \
			$(top_builddir)/util/symlinks

#
# Warning flags
#
# Run make gcc-wall to do a build with warning messages.
#
#
WFLAGS=		-std=gnu99 -D_XOPEN_SOURCE=600 -D_GNU_SOURCE $(WFLAGS_EXTRA) \
			-Wall -W -Wwrite-strings -Wpointer-arith \
			-Wcast-qual -Wcast-align -Wno-variadic-macros \
			-Wstrict-prototypes -Wmissing-prototypes \
			-Wformat-security  -Wformat-nonliteral \
			-Wmissing-format-attribute -O2 -Wstrict-aliasing \
			-Wnested-externs -Winline -DNO_INLINE_FUNCS -Wshadow \
			-UENABLE_NLS

gcc-wall-new:
	($(MAKE) CFLAGS_WARN="$(WFLAGS)" > /dev/null) 2>&1

gcc-wall:
	$(MAKE) clean > /dev/null
	$(MAKE) gcc-wall-new

static-check:
	($(MAKE) C=1 V=1 CFLAGS="$(ALL_CFLAGS) $(WFLAGS)") 2>&1

static-check-all:
	$(MAKE) clean > /dev/null
	$(MAKE) static-check

#
# Installation user and groups
#
BINGRP=		bin
BINOWN=		bin
BINMODE=	555
INCGRP=		bin
INCOWN=		bin
INCMODE=	444
LIBOWN=		bin
LIBGRP=		bin
LIBMODE=	444
MANGRP=		bin
MANOWN=		bin
MANMODE=	444

#
# Autoconf magic...
#

DEP_LIB_MAKEFILES = $(top_srcdir)/lib/Makefile.library \
	$(top_srcdir)/lib/Makefile.elf-lib  \
	$(top_srcdir)/lib/Makefile.bsd-lib \
	$(top_srcdir)/lib/Makefile.darwin-lib \
	$(top_srcdir)/lib/Makefile.solaris-lib \
	$(top_srcdir)/lib/Makefile.profile

$(top_builddir)/config.status: $(top_srcdir)/configure
	cd $(top_builddir); ./config.status --recheck

$(top_builddir)/MCONFIG: $(top_srcdir)/MCONFIG.in $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=MCONFIG ./config.status

$(top_builddir)/lib/config.h: $(top_srcdir)/lib/config.h.in \
			      $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=lib/config.h ./config.status

$(top_builddir)/lib/dirpaths.h: $(DEP_SUBSTITUTE) $(top_srcdir)/lib/dirpaths.h.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE) $(top_srcdir)/lib/dirpaths.h.in $@

$(top_builddir)/lib/substitute_sh: $(top_srcdir)/lib/substitute_sh.in \
		$(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=lib/substitute_sh ./config.status

$(top_builddir)/util/subst.conf: $(top_srcdir)/util/subst.conf.in \
		$(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=util/subst.conf ./config.status

Makefile: $(srcdir)/Makefile.in $(top_builddir)/MCONFIG \
		$(DEP_MAKEFILE) $(top_builddir)/config.status
	cd $(top_builddir); CONFIG_FILES=$(my_dir)/Makefile ./config.status

#$(top_srcdir)/configure: $(top_srcdir)/configure.ac
#	cd $(top_srcdir) && autoheader && autoconf

coverage.txt: Makefile $(SRCS)
	if test -n "$(SRCS)"; then \
		gcov -s $(top_srcdir) -o . $(SRCS) > coverage.txt 2>&1 ; \
	fi

clean::
	$(RM) -f *.gcda *.gcov *.gcno coverage.txt

#
# Make depend magic...
#

.depend: Makefile $(SRCS) $(top_srcdir)/depfix.sed $(top_srcdir)/wordwrap.pl
	if test -n "$(SRCS)" ; then \
		$(CC) -M $(ALL_CFLAGS) $(DEPEND_CFLAGS) $(SRCS) | \
			$(SED) -f $(top_srcdir)/depfix.sed \
			    -e 's; $(srcdir)/; $$(srcdir)/;g' \
			    -e 's; $(top_srcdir)/; $$(top_srcdir)/;g' \
			    -e 's; $(top_builddir)/; $$(top_builddir)/;g' \
			    -e 's; \./; ;g' \
			    -e '/^#/d' \
			    -e '/^ *\\$$/d' | \
			$(PERL) $(top_srcdir)/wordwrap.pl > .depend; \
	else :; fi

depend:: .depend
	if test -n "$(SRCS)" ; then \
		sed -e '/^# +++ Dependency line eater +++/,$$d' \
			< $(srcdir)/Makefile.in | cat - .depend \
			> $(srcdir)/Makefile.in.new; \
	if cmp -s $(srcdir)/Makefile.in $(srcdir)/Makefile.in.new ; then \
		$(RM) $(srcdir)/Makefile.in.new ; \
	else \
		$(MV) $(srcdir)/Makefile.in $(srcdir)/Makefile.in.old; \
		$(MV) $(srcdir)/Makefile.in.new $(srcdir)/Makefile.in; \
	fi ; else :; fi

# End of file MCONFIG

PROGS=		e2scrub e2scrub_all
MANPAGES=	e2scrub.8 e2scrub_all.8
CONFFILES=	e2scrub.conf

ifeq ($(HAVE_UDEV),yes)
UDEV_RULES	= e2scrub.rules
INSTALLDIRS_TGT	+= installdirs-udev
INSTALL_TGT	+= install-udev
UNINSTALL_TGT	+= uninstall-udev
endif

ifeq ($(HAVE_CROND),yes)
CRONTABS	= e2scrub_all.cron
LIBPROGS	+= e2scrub_all_cron
INSTALLDIRS_TGT	+= installdirs-crond installdirs-libprogs
INSTALL_TGT	+= install-crond install-libprogs
UNINSTALL_TGT	+= uninstall-crond uninstall-libprogs
endif

ifeq ($(HAVE_SYSTEMD),yes)
SERVICE_FILES	= e2scrub@.service e2scrub_all.service e2scrub_all.timer e2scrub_fail@.service e2scrub_reap.service
LIBPROGS	+= e2scrub_fail
INSTALLDIRS_TGT	+= installdirs-systemd installdirs-libprogs
INSTALL_TGT	+= install-systemd install-libprogs
UNINSTALL_TGT	+= uninstall-systemd uninstall-libprogs
endif

all:: $(PROGS) $(MANPAGES) $(CONFFILES) $(UDEV_RULES) $(SERVICE_FILES) $(CRONTABS) $(LIBPROGS)

e2scrub: $(DEP_SUBSTITUTE) e2scrub.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $(srcdir)/e2scrub.in $@
	$(Q) chmod a+x $@

e2scrub_all: e2scrub_all.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $(srcdir)/e2scrub_all.in $@
	$(Q) chmod a+x $@

e2scrub_fail: e2scrub_fail.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $(srcdir)/e2scrub_fail.in $@
	$(Q) chmod a+x $@

e2scrub_all_cron: e2scrub_all_cron.in
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $(srcdir)/e2scrub_all_cron.in $@
	$(Q) chmod a+x $@

%.8: %.8.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

%.conf: %.conf.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

%.rules: %.rules.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

%.service: %.service.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

%.cron: %.cron.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

%.timer: %.timer.in $(DEP_SUBSTITUTE)
	$(E) "	SUBST $@"
	$(Q) $(SUBSTITUTE_UPTIME) $< $@

installdirs-udev:
	$(E) "	MKDIR_P $(UDEV_RULES_DIR)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(UDEV_RULES_DIR)

installdirs-crond:
	$(E) "	MKDIR_P $(CROND_DIR)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(CROND_DIR)

installdirs-libprogs:
	$(E) "	MKDIR_P $(pkglibdir)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(pkglibdir)

installdirs-systemd:
	$(E) "	MKDIR_P $(SYSTEMD_SYSTEM_UNIT_DIR)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(SYSTEMD_SYSTEM_UNIT_DIR)

installdirs: $(INSTALLDIRS_TGT)
	$(E) "	MKDIR_P $(root_sbindir) $(man8dir) $(root_sysconfdir)"
	$(Q) $(MKDIR_P) $(DESTDIR)$(root_sbindir) \
		$(DESTDIR)$(man8dir) $(DESTDIR)$(root_sysconfdir)

install-udev: installdirs-udev
	$(Q) for i in $(UDEV_RULES); do \
		$(ES) "	INSTALL $(UDEV_RULES_DIR)/$$i"; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(UDEV_RULES_DIR)/96-$$i; \
	done

install-crond: installdirs-crond
	$(Q) if test -n "$(CRONTABS)" ; then \
		$(ES) "	INSTALL $(CROND_DIR)/e2scrub_all" ; \
		$(INSTALL_DATA) e2scrub_all.cron $(DESTDIR)$(CROND_DIR)/e2scrub_all ; \
	fi

install-libprogs: $(LIBPROGS) installdirs-libprogs
	$(Q) for i in $(LIBPROGS); do \
		$(ES) "	INSTALL $(pkglibdir)/$$i"; \
		$(INSTALL_PROGRAM) $$i $(DESTDIR)$(pkglibdir)/$$i; \
	done

install-systemd: $(SERVICE_FILES) installdirs-systemd
	$(Q) for i in $(SERVICE_FILES); do \
		$(ES) "	INSTALL_DATA $(SYSTEMD_SYSTEM_UNIT_DIR)/$$i"; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(SYSTEMD_SYSTEM_UNIT_DIR)/$$i; \
	done

install-strip: install

install: $(PROGS) $(MANPAGES) $(FMANPAGES) installdirs $(INSTALL_TGT)
	$(Q) for i in $(PROGS); do \
		$(ES) "	INSTALL $(root_sbindir)/$$i"; \
		$(INSTALL_PROGRAM) $$i $(DESTDIR)$(root_sbindir)/$$i; \
	done
	$(Q) for i in $(MANPAGES); do \
		for j in $(COMPRESS_EXT); do \
			$(RM) -f $(DESTDIR)$(man8dir)/$$i.$$j; \
		done; \
		$(ES) "	INSTALL_DATA $(man8dir)/$$i"; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(man8dir)/$$i; \
	done
	$(Q) for i in $(CONFFILES); do \
		$(ES) "	INSTALL_DATA $(root_sysconfdir)/$$i"; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(root_sysconfdir)/$$i; \
	done

uninstall-udev:
	for i in $(UDEV_RULES); do \
		$(RM) -f $(DESTDIR)$(UDEV_RULES_DIR)/96-$$i; \
	done

uninstall-crond:
	if test -n "$(CRONTABS)" ; then \
		$(RM) -f $(DESTDIR)$(CROND_DIR)/e2scrub_all ; \
	fi

uninstall-libprogs:
	for i in $(LIBPROGS); do \
		$(RM) -f $(DESTDIR)$(pkglibdir)/$$i; \
	done

uninstall-systemd:
	for i in $(SERVICE_FILES); do \
		$(RM) -f $(DESTDIR)$(SYSTEMD_SYSTEM_UNIT_DIR)/$$i; \
	done

uninstall: $(UNINSTALL_TGT)
	for i in $(PROGS); do \
		$(RM) -f $(DESTDIR)$(root_sbindir)/$$i; \
	done
	for i in $(MANPAGES); do \
		$(RM) -f $(DESTDIR)$(man8dir)/$$i; \
	done
	for i in $(CONFFILES); do \
		$(RM) -f $(DESTDIR)$(root_sysconfdir)/$$i; \
	done

clean::
	$(RM) -f $(PROGS) $(MANPAGES) $(CONFFILES) $(UDEV_RULES) $(SERVICE_FILES) $(CRONTABS) $(LIBPROGS)

mostlyclean: clean
distclean: clean
	$(RM) -f .depend Makefile $(srcdir)/TAGS $(srcdir)/Makefile.in.old
