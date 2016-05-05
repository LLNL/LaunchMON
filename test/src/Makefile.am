## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##
##--------------------------------------------------------------------------------
## Copyright (c) 2008 ~ 2012, Lawrence Livermore National Security, LLC. Produced at
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
##        Apr 28 2016 DHA: Modernize autotools and build environments
##        Aug 03 2010 DHA: Added LMONHOSTLIST support
##        May 29 2009 DHA: Added fe_regErrorCB.in test
##        Jun 12 2008 DHA: Created file.
##

MPICC = @MPICC@
SMPFACTOR = @SMPFACTOR@
TESTNNODES = @TESTNNODES@
TARGET_JOB_LAUNCHER_PATH = @TARGET_JOB_LAUNCHER_PATH@
RM_TYPE = @RM_TYPE@
LMONHOSTLIST = @LMONHL@
PLAT_SRC_DIR = $(abs_top_srcdir)/launchmon/src/linux
API_SRC_DIR = $(abs_top_srcdir)/launchmon/src/linux/lmon_api
BASE_SRC_DIR = ${abs_top_srcdir}/launchmon/src

if WITH_CIOD
CIOD_INC_FLAGS = -I@ARCHHEADER@ \
                 -I$(abs_top_srcdir)/@CIODLOC@
endif

if WITH_TEST_INSTALLED
API_DIR = $(prefix)/include
API_LIB_DIR = $(prefix)/lib
ENGINE_PATH = $(prefix)/bin/launchmon$(EXEEXT)
LMONPREFIX = $(prefix)
IN_TREE_RM_CONFIG_DIR = 0
IN_TREE_COLOC_UTIL_DIR = 0
else
API_DIR = ${abs_top_srcdir}/launchmon/src
API_LIB_DIR = $(abs_top_builddir)/launchmon/src/linux/lmon_api
ENGINE_PATH = $(abs_top_builddir)/launchmon/src/linux/launchmon$(EXEEXT)
LMONPREFIX = 0
IN_TREE_RM_CONFIG_DIR = $(abs_top_srcdir)/etc
IN_TREE_COLOC_UTIL_DIR = $(abs_top_builddir)/tools/alps/src
endif

AM_CPPFLAGS = \
  -I$(API_DIR) \
  -I$(BASE_SRC_DIR) \
  -I$(PLAT_SRC_DIR) \
  $(CIOD_INC_FLAGS) \
  -I$(API_SRC_DIR)

testdir = ${pkgdatadir}/tests
check_PROGRAMS = \
  fe_jobsnap \
  be_jobsnap \
  LE_model_checker \
  fe_launch_smoketest \
  fe_launch_usrpayload_test \
  fe_launch_middleware \
  fe_attach_smoketest \
  be_kicker \
  be_mem_fetcher \
  be_kicker_usrpayload_test \
  be_standalone_kicker \
  mw_comm_helper \
  run_3mins \
  hang_on_SIGUSR1 \
  simple_MPI

LE_model_checker_SOURCES = LE_model_checker.c
LE_model_checker_CFLAGS = @LNCHR_BIT_FLAGS@ $(AM_CFLAGS)
LE_model_checker_CXXFLAGS = @LNCHR_BIT_FLAGS@ $(AM_CXXFLAGS)
LE_model_checker_LDADD = @LIBPTHREAD@
LE_model_checker_LDFLAGS = @LNCHR_BIT_FLAGS@ @LIBPTHREAD@ $(AM_LDFLAGS)

fe_launch_smoketest_SOURCES = fe_launch_smoketest.cxx util.c
fe_launch_smoketest_CFLAGS = $(AM_CFLAGS)
fe_launch_smoketest_CXXFLAGS = $(AM_CXXFLAGS)
fe_launch_smoketest_LDFLAGS = -L$(API_LIB_DIR)
fe_launch_smoketest_LDADD = -lmonfeapi 

fe_launch_usrpayload_test_SOURCES = fe_launch_usrpayload_test.cxx util.c
fe_launch_usrpayload_test_CFLAGS = $(AM_CFLAGS)
fe_launch_usrpayload_test_CXXFLAGS = $(AM_CXXFLAGS)
fe_launch_usrpayload_test_LDFLAGS = -L$(API_LIB_DIR)
fe_launch_usrpayload_test_LDADD = -lmonfeapi 

fe_attach_smoketest_SOURCES = fe_attach_smoketest.cxx util.c
fe_attach_smoketest_CFLAGS = $(AM_CFLAGS)
fe_attach_smoketest_CXXFLAGS = $(AM_CXXFLAGS)
fe_attach_smoketest_LDFLAGS = -L$(API_LIB_DIR)
fe_attach_smoketest_LDADD = -lmonfeapi

be_kicker_SOURCES = be_kicker.cxx util.c
be_kicker_CFLAGS = $(AM_CFLAGS)
be_kicker_CXXFLAGS = $(AM_CXXFLAGS)
be_kicker_LDFLAGS = -L$(API_LIB_DIR)
be_kicker_LDADD = -lmonbeapi 

be_kicker_usrpayload_test_SOURCES = be_kicker_usrpayload_test.cxx
be_kicker_usrpayload_test_CFLAGS = $(AM_CFLAGS)
be_kicker_usrpayload_test_CXXFLAGS = $(AM_CXXFLAGS)
be_kicker_usrpayload_test_LDFLAGS = -L$(API_LIB_DIR)
be_kicker_usrpayload_test_LDADD = -lmonbeapi

fe_launch_middleware_SOURCES = fe_launch_middleware.cxx util.c
fe_launch_middleware_CFLAGS = $(AM_CFLAGS)
fe_launch_middleware_CXXFLAGS = $(AM_CXXFLAGS)
fe_launch_middleware_LDFLAGS = -L$(API_LIB_DIR)
fe_launch_middleware_LDADD = -lmonfeapi 

be_standalone_kicker_SOURCES = be_standalone_kicker.cxx
be_standalone_kicker_CFLAGS = $(AM_CFLAGS)
be_standalone_kicker_CXXFLAGS = $(AM_CXXFLAGS)
be_standalone_kicker_LDFLAGS = -L$(API_LIB_DIR)
be_standalone_kicker_LDADD = -lmonbeapi

be_mem_fetcher_SOURCES = be_mem_fetcher.cxx util.c
be_mem_fetcher_CFLAGS = $(AM_CFLAGS)
be_mem_fetcher_CXXFLAGS = $(AM_CXXFLAGS)
be_mem_fetcher_LDFLAGS = -L$(API_LIB_DIR)
be_mem_fetcher_LDADD = -lmonbeapi 

fe_jobsnap_SOURCES = fe_jobsnap.cxx util.c
fe_jobsnap_CFLAGS = $(AM_CFLAGS)
fe_jobsnap_CXXFLAGS = $(AM_CXXFLAGS)
fe_jobsnap_LDFLAGS = -L$(API_LIB_DIR)
fe_jobsnap_LDADD = -lmonfeapi 

be_jobsnap_SOURCES = be_jobsnap.cxx util.c
be_jobsnap_CFLAGS = $(AM_CFLAGS)
be_jobsnap_CXXFLAGS = $(AM_CXXFLAGS)
be_jobsnap_LDFLAGS = -L$(API_LIB_DIR)
be_jobsnap_LDADD = -lmonbeapi

mw_comm_helper_SOURCES = mw_comm_helper.cxx util.c
mw_comm_helper_CFLAGS = $(AM_CFLAGS)
mw_comm_helper_CXXFLAGS = $(AM_CXXFLAGS)
mw_comm_helper_LDFLAGS = -L$(API_LIB_DIR)
mw_comm_helper_LDADD = -lmonmwapi

test_input_files =\
  test.LE_model_checker_mainthread.in \
  test.LE_model_checker_pthread.in \
  test.LE_model_checker_large.in \
  test.LE_model_checker_extreme.in \
  test.launch_1.in \
  test.launch_1_remote.in \
  test.launch_2_half.in \
  test.launch_2_uneven.in \
  test.launch_3_invalid_dmonpath.in \
  test.launch_5_usrpayload.in \
  test.launch_6_engine_failure.in \
  test.launch_7_kill.in \
  test.launch_7_shutdownbe.in \
  test.launch_7_detach.in \
  test.launch_mw_1_hostlist.in \
  test.launch_mw_2_coloc.in \
  test.launch_mw_5_mixall.in \
  test.attach_1.in \
  test.attach_1_mem_fetcher.in \
  test.attach_1_remote.in \
  test.attach_1_pdebugmax.in \
  test.attach_2_uneven.in \
  test.attach_3_invalidpid.in \
  test.attach_3_invalidpid_remote.in \
  test.attach_3_callsafterfail.in \
  test.attach_4_kill.in \
  test.attach_4_shutdownbe.in \
  test.attach_4_detach.in \
  test.jobsnap_1.in \
  test.fe_regErrorCB.in \
  test.fe_regStatusCB.in

check_SCRIPTS = $(test_input_files:.in=)
CLEANFILES = $(check_SCRIPTS)
EXTRA_DIST = $(test_input_files)

do_subst = \
    sed -e 's,@LMON@,$(ENGINE_PATH),g' \
        -e 's,@LMONMWHL@,$(LMONHOSTLIST),g' \
	    -e 's,@LMONPREFIX@,$(LMONPREFIX),g' \
	    -e 's,@RMCONFIGDIR@,$(IN_TREE_RM_CONFIG_DIR),g' \
	    -e 's,@COLOCDIR@,$(IN_TREE_COLOC_UTIL_DIR),g' \
        -e 's,@TJLPATH@,$(TARGET_JOB_LAUNCHER_PATH),g' \
        -e 's,@TEST_RMTP@,$(RM_TYPE),g' \
	    -e 's,@SMP@,$(SMPFACTOR),g' \
	    -e 's,@NNODES@,$(TESTNNODES),g' \
        -e 's,@EXE@,$(EXEEXT),g'

$(check_SCRIPTS): %: %.in
	$(do_subst) < $< > $@
	chmod +x $@

run_3mins$(EXEEXT): run_3mins.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) -I$(BASE_SRC_DIR) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	

hang_on_SIGUSR1$(EXEEXT): hang_on_SIGUSR1.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) -I$(BASE_SRC_DIR) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	

simple_MPI$(EXEEXT): simple_MPI.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) -I$(BASE_SRC_DIR) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	

