## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##--------------------------------------------------------------------------------
## Copyright (c) 2008, Lawrence Livermore National Security, LLC. Produced at
## the Lawrence Livermore National Laboratory. Written by Dong H. Ahn <ahn1@llnl.gov>.
## LLNL-CODE-409469. All rights reserved.
##
## This file is part of LaunchMON. For details, see
## https://computing.llnl.gov/?set=resources&page=os_projects
##
## Please also read LICENSE -- Our Notice and GNU Lesser General Public License.
##
##
## This program is free software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License (as published by the Free Software
## Foundation) version 2.1 dated February 1999.
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License along
## with this program; if not, write to the Free Software Foundation, Inc., 59 Temple
## Place, Suite 330, Boston, MA 02111-1307 USA
##--------------------------------------------------------------------------------
##
##  Update Log:
##        Dec 16 2009 DHA: Added COBO support
##        Jun 10 2007 DHA: Copied from the old Makefile.
##

if WITH_PMGR_COLLECTIVE
  OPT_PMGR_COLLECTIVE = pmgr_collective
endif 
if WITH_COBO
  OPT_COBO = cobo
endif
if WITH_CIOD
  OPT_CIOD = ciod
endif
if WITH_ALPS
  OPT_ALPS = alps
endif
  OPT_GCRYPT = libgpg-error

SUBDIRS = $(OPT_PMGR_COLLECTIVE) $(OPT_COBO) $(OPT_CIOD) $(OPT_ALPS) $(OPT_GCRYPT)

all-local:
	if test -f libgcrypt/Makefile; then \
                export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && $(MAKE) $(AM_MAKEFILE) all; \
	else \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && ./configure --prefix=$(prefix) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LD="$(LD)" && $(MAKE) $(AM_MAKEFILE) all; \
	fi


install-exec-local:
	if test -f libgcrypt/Makefile; then \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && $(MAKE) $(AM_MAKEFILE) install-exec; \
	else \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && ./configure --prefix=$(prefix) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LD="$(LD)" && $(MAKE) $(AM_MAKEFILE) install-exec; \
	fi
 
 
install-data-local:
	if test -f libgcrypt/Makefile; then \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && $(MAKE) $(AM_MAKEFILE) install-data; \
	else \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && ./configure --prefix=$(prefix) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LD="$(LD)" && $(MAKE) $(AM_MAKEFILE) install-data; \
	fi
 
 
clean-local:
	if test -f libgcrypt/Makefile; then \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && $(MAKE) $(AM_MAKEFILE) clean; \
	else \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && ./configure --prefix=$(prefix) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LD="$(LD)" && $(MAKE) $(AM_MAKEFILE) clean; \
        fi

distclean-local:
	if test -f libgcrypt/Makefile; then \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && $(MAKE) $(AM_MAKEFILE) distclean; \
	else \
		export GPG_ERROR_CONFIG=`$(top_srcdir)/config/ap $(top_srcdir)/tools/libgpg-error/src/gpg-error-config` \
		&& cd libgcrypt && ./configure --prefix=$(prefix) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" LD="$(LD)" && $(MAKE) $(AM_MAKEFILE) distclean; \
        fi

LIBGCRYPTDISTFILES = libgcrypt/src libgcrypt/src/gcrypt.h libgcrypt/src/gcrypt-module.h libgcrypt/src/Makefile.in libgcrypt/src/ChangeLog libgcrypt/src/Makefile.am libgcrypt/src/libgcrypt-config.in libgcrypt/src/g10lib.h libgcrypt/src/types.h libgcrypt/src/cipher.h libgcrypt/src/misc.c libgcrypt/src/global.c libgcrypt/src/sexp.c libgcrypt/src/stdmem.c libgcrypt/src/stdmem.h libgcrypt/src/secmem.c libgcrypt/src/secmem.h libgcrypt/src/mpi.h libgcrypt/src/missing-string.c libgcrypt/src/module.c libgcrypt/src/ath.h libgcrypt/src/ath.c libgcrypt/src/Manifest libgcrypt/src/libgcrypt.m4 libgcrypt/src/libgcrypt.vers libgcrypt/README libgcrypt/Makefile.in libgcrypt/configure libgcrypt/AUTHORS libgcrypt/COPYING libgcrypt/COPYING.LIB libgcrypt/ChangeLog libgcrypt/INSTALL libgcrypt/Makefile.am libgcrypt/NEWS libgcrypt/THANKS libgcrypt/TODO libgcrypt/acinclude.m4 libgcrypt/aclocal.m4 libgcrypt/compile libgcrypt/config.guess libgcrypt/config.h.in libgcrypt/config.sub libgcrypt/configure.ac libgcrypt/depcomp libgcrypt/install-sh libgcrypt/ltmain.sh libgcrypt/missing libgcrypt/mkinstalldirs libgcrypt/BUGS libgcrypt/autogen.sh libgcrypt/README.apichanges libgcrypt/README.CVS libgcrypt/mpi libgcrypt/mpi/Makefile.in libgcrypt/mpi/ChangeLog libgcrypt/mpi/Makefile.am libgcrypt/mpi/longlong.h libgcrypt/mpi/mpi-add.c libgcrypt/mpi/mpi-bit.c libgcrypt/mpi/mpi-cmp.c libgcrypt/mpi/mpi-div.c libgcrypt/mpi/mpi-gcd.c libgcrypt/mpi/mpi-internal.h libgcrypt/mpi/mpi-inline.h libgcrypt/mpi/mpi-inline.c libgcrypt/mpi/mpi-inv.c libgcrypt/mpi/mpi-mul.c libgcrypt/mpi/mpi-pow.c libgcrypt/mpi/mpi-mpow.c libgcrypt/mpi/mpi-scan.c libgcrypt/mpi/mpicoder.c libgcrypt/mpi/mpih-div.c libgcrypt/mpi/mpih-mul.c libgcrypt/mpi/mpiutil.c libgcrypt/mpi/Manifest libgcrypt/mpi/config.links libgcrypt/mpi/alpha libgcrypt/mpi/alpha/distfiles libgcrypt/mpi/alpha/README libgcrypt/mpi/alpha/mpih-add1.S libgcrypt/mpi/alpha/mpih-sub1.S libgcrypt/mpi/alpha/mpih-mul1.S libgcrypt/mpi/alpha/mpih-mul2.S libgcrypt/mpi/alpha/mpih-mul3.S libgcrypt/mpi/alpha/mpih-lshift.S libgcrypt/mpi/alpha/mpih-rshift.S libgcrypt/mpi/alpha/udiv-qrnnd.S libgcrypt/mpi/generic libgcrypt/mpi/generic/distfiles libgcrypt/mpi/generic/Manifest libgcrypt/mpi/generic/mpih-add1.c libgcrypt/mpi/generic/mpih-mul1.c libgcrypt/mpi/generic/mpih-mul2.c libgcrypt/mpi/generic/mpih-mul3.c libgcrypt/mpi/generic/mpih-lshift.c libgcrypt/mpi/generic/mpih-rshift.c libgcrypt/mpi/generic/mpih-sub1.c libgcrypt/mpi/generic/udiv-w-sdiv.c libgcrypt/mpi/generic/mpi-asm-defs.h libgcrypt/mpi/hppa libgcrypt/mpi/hppa/distfiles libgcrypt/mpi/hppa/README libgcrypt/mpi/hppa/udiv-qrnnd.S libgcrypt/mpi/hppa/mpih-add1.S libgcrypt/mpi/hppa/mpih-sub1.S libgcrypt/mpi/hppa/mpih-lshift.S libgcrypt/mpi/hppa/mpih-rshift.S libgcrypt/mpi/hppa1.1 libgcrypt/mpi/hppa1.1/distfiles libgcrypt/mpi/hppa1.1/udiv-qrnnd.S libgcrypt/mpi/hppa1.1/mpih-mul1.S libgcrypt/mpi/hppa1.1/mpih-mul2.S libgcrypt/mpi/hppa1.1/mpih-mul3.S libgcrypt/mpi/i386 libgcrypt/mpi/i386/distfiles libgcrypt/mpi/i386/Manifest libgcrypt/mpi/i386/mpih-add1.S libgcrypt/mpi/i386/mpih-mul1.S libgcrypt/mpi/i386/mpih-mul2.S libgcrypt/mpi/i386/mpih-mul3.S libgcrypt/mpi/i386/mpih-lshift.S libgcrypt/mpi/i386/mpih-rshift.S libgcrypt/mpi/i386/mpih-sub1.S libgcrypt/mpi/i386/syntax.h libgcrypt/mpi/i586 libgcrypt/mpi/i586/distfiles libgcrypt/mpi/i586/Manifest libgcrypt/mpi/i586/mpih-add1.S libgcrypt/mpi/i586/mpih-mul1.S libgcrypt/mpi/i586/mpih-mul2.S libgcrypt/mpi/i586/mpih-mul3.S libgcrypt/mpi/i586/mpih-lshift.S libgcrypt/mpi/i586/mpih-rshift.S libgcrypt/mpi/i586/mpih-sub1.S libgcrypt/mpi/i586/README libgcrypt/mpi/m68k libgcrypt/mpi/m68k/mc68020 libgcrypt/mpi/m68k/mc68020/distfiles libgcrypt/mpi/m68k/mc68020/Manifest libgcrypt/mpi/m68k/mc68020/mpih-mul1.S libgcrypt/mpi/m68k/mc68020/mpih-mul2.S libgcrypt/mpi/m68k/mc68020/mpih-mul3.S libgcrypt/mpi/m68k/distfiles libgcrypt/mpi/m68k/Manifest libgcrypt/mpi/m68k/syntax.h libgcrypt/mpi/m68k/mpih-lshift.S libgcrypt/mpi/m68k/mpih-rshift.S libgcrypt/mpi/m68k/mpih-add1.S libgcrypt/mpi/m68k/mpih-sub1.S libgcrypt/mpi/mips3 libgcrypt/mpi/mips3/distfiles libgcrypt/mpi/mips3/Manifest libgcrypt/mpi/mips3/README libgcrypt/mpi/mips3/mpih-add1.S libgcrypt/mpi/mips3/mpih-sub1.S libgcrypt/mpi/mips3/mpih-mul1.S libgcrypt/mpi/mips3/mpih-mul2.S libgcrypt/mpi/mips3/mpih-mul3.S libgcrypt/mpi/mips3/mpih-lshift.S libgcrypt/mpi/mips3/mpih-rshift.S libgcrypt/mpi/mips3/mpi-asm-defs.h libgcrypt/mpi/pa7100 libgcrypt/mpi/pa7100/distfiles libgcrypt/mpi/pa7100/Manifest libgcrypt/mpi/pa7100/mpih-lshift.S libgcrypt/mpi/pa7100/mpih-rshift.S libgcrypt/mpi/power libgcrypt/mpi/power/distfiles libgcrypt/mpi/power/Manifest libgcrypt/mpi/power/mpih-add1.S libgcrypt/mpi/power/mpih-lshift.S libgcrypt/mpi/power/mpih-mul1.S libgcrypt/mpi/power/mpih-mul2.S libgcrypt/mpi/power/mpih-mul3.S libgcrypt/mpi/power/mpih-rshift.S libgcrypt/mpi/power/mpih-sub1.S libgcrypt/mpi/powerpc32 libgcrypt/mpi/powerpc32/distfiles libgcrypt/mpi/powerpc32/Manifest libgcrypt/mpi/powerpc32/mpih-add1.S libgcrypt/mpi/powerpc32/mpih-sub1.S libgcrypt/mpi/powerpc32/mpih-mul1.S libgcrypt/mpi/powerpc32/mpih-mul2.S libgcrypt/mpi/powerpc32/mpih-mul3.S libgcrypt/mpi/powerpc32/mpih-lshift.S libgcrypt/mpi/powerpc32/mpih-rshift.S libgcrypt/mpi/powerpc32/syntax.h libgcrypt/mpi/powerpc64 libgcrypt/mpi/powerpc64/distfiles libgcrypt/mpi/sparc32 libgcrypt/mpi/sparc32/distfiles libgcrypt/mpi/sparc32/Manifest libgcrypt/mpi/sparc32/mpih-lshift.S libgcrypt/mpi/sparc32/mpih-rshift.S libgcrypt/mpi/sparc32/mpih-add1.S libgcrypt/mpi/sparc32/udiv.S libgcrypt/mpi/sparc32v8 libgcrypt/mpi/sparc32v8/distfiles libgcrypt/mpi/sparc32v8/Manifest libgcrypt/mpi/sparc32v8/mpih-mul1.S libgcrypt/mpi/sparc32v8/mpih-mul2.S libgcrypt/mpi/sparc32v8/mpih-mul3.S libgcrypt/mpi/supersparc libgcrypt/mpi/supersparc/distfiles libgcrypt/mpi/supersparc/Manifest libgcrypt/mpi/supersparc/udiv.S libgcrypt/cipher libgcrypt/cipher/Makefile.in libgcrypt/cipher/ChangeLog libgcrypt/cipher/Makefile.am libgcrypt/cipher/cipher.c libgcrypt/cipher/pubkey.c libgcrypt/cipher/ac.c libgcrypt/cipher/md.c libgcrypt/cipher/bithelp.h libgcrypt/cipher/primegen.c libgcrypt/cipher/random.c libgcrypt/cipher/random.h libgcrypt/cipher/rand-internal.h libgcrypt/cipher/rmd.h libgcrypt/cipher/arcfour.c libgcrypt/cipher/blowfish.c libgcrypt/cipher/cast5.c libgcrypt/cipher/crc.c libgcrypt/cipher/des.c libgcrypt/cipher/dsa.c libgcrypt/cipher/elgamal.c libgcrypt/cipher/md4.c libgcrypt/cipher/md5.c libgcrypt/cipher/rijndael.c libgcrypt/cipher/rmd160.c libgcrypt/cipher/rndlinux.c libgcrypt/cipher/rndegd.c libgcrypt/cipher/rndunix.c libgcrypt/cipher/rndw32.c libgcrypt/cipher/rsa.c libgcrypt/cipher/serpent.c libgcrypt/cipher/sha1.c libgcrypt/cipher/sha256.c libgcrypt/cipher/sha512.c libgcrypt/cipher/tiger.c libgcrypt/cipher/twofish.c libgcrypt/cipher/rfc2268.c libgcrypt/cipher/Manifest libgcrypt/doc libgcrypt/doc/lgpl.texi libgcrypt/doc/gpl.texi libgcrypt/doc/Makefile.in libgcrypt/doc/ChangeLog libgcrypt/doc/Makefile.am libgcrypt/doc/mdate-sh libgcrypt/doc/stamp-vti libgcrypt/doc/texinfo.tex libgcrypt/doc/version.texi libgcrypt/doc/gcrypt.texi libgcrypt/doc/gcrypt.info libgcrypt/tests libgcrypt/tests/Makefile.in libgcrypt/tests/ChangeLog libgcrypt/tests/Makefile.am libgcrypt/tests/ac.c libgcrypt/tests/basic.c libgcrypt/tests/benchmark.c libgcrypt/tests/keygen.c libgcrypt/tests/prime.c libgcrypt/tests/pubkey.c libgcrypt/tests/register.c libgcrypt/tests/testapi.c libgcrypt/tests/tsexp.c libgcrypt/w32-dll libgcrypt/w32-dll/Makefile.in libgcrypt/w32-dll/ChangeLog libgcrypt/w32-dll/Makefile.am libgcrypt/VERSION 

		export gpg_config_path=`$(top_srcdir)/config/ac ../libgpg-error/src/gpg-error-config`; 
 EXTRA_DIST = $(LIBGCRYPTDISTFILES)
