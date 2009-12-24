## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
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
##        May 29 2009 DHA: Added fe_regErrorCB.in test
##        Jun 12 2008 DHA: Created file.
##

MPICC                             = @MPICC@
SMPFACTOR                         = @SMPFACTOR@
TESTNNODES                        = @TESTNNODES@
TARGET_JOB_LAUNCHER_PATH          = @TARGET_JOB_LAUNCHER_PATH@
RM_TYPE                           = @RM_TYPE@

INCLUDES                          = -I$(top_srcdir)/launchmon/src -I$(top_srcdir)/@LMONAPILOC@
INCLUDES                          += -I$(top_srcdir)/@GCRYPTLOC@ -I$(top_srcdir)/@GPGERRLOC@
if WITH_CIOD
INCLUDES                          += -I@ARCHHEADER@ -I$(top_srcdir)/@CIODLOC@
endif

bin_PROGRAMS                      = fe_jobsnap \
				    be_jobsnap \
				    LE_model_checker \
				    fe_launch_smoketest \
				    fe_launch_usrpayload_test \
			   	    fe_attach_smoketest \
				    be_kicker \
				    be_kicker_usrpayload_test \
				    be_standalone_kicker \
				    run_3mins \
				    hang_on_SIGUSR1 \
				    simple_MPI 

LE_model_checker_SOURCES          = LE_model_checker.c
LE_model_checker_CFLAGS           = @BUILD_FLAGS@ @LNCHR_BIT_FLAGS@ $(AM_CFLAGS)
LE_model_checker_CXXFLAGS         = @BUILD_FLAGS@ @LNCHR_BIT_FLAGS@ $(AM_CXXFLAGS)
LE_model_checker_LDADD            = @LIBPTHREAD@
LE_model_checker_LDFLAGS          = @LNCHR_BIT_FLAGS@ @LIBPTHREAD@ $(AM_LDFLAGS)

fe_launch_smoketest_SOURCES       = fe_launch_smoketest.cxx util.c
fe_launch_smoketest_CFLAGS        = @BUILD_FLAGS@ $(AM_CFLAGS)
fe_launch_smoketest_CXXFLAGS      = @BUILD_FLAGS@ $(AM_CXXFLAGS)
fe_launch_smoketest_LDFLAGS       = -L$(top_srcdir)/@LMONAPILOC@ 
fe_launch_smoketest_LDADD         = -lmonfeapi 

fe_launch_usrpayload_test_SOURCES = fe_launch_usrpayload_test.cxx util.c
fe_launch_usrpayload_test_CFLAGS   = @BUILD_FLAGS@ $(AM_CFLAGS)
fe_launch_usrpayload_test_CXXFLAGS = @BUILD_FLAGS@ $(AM_CXXFLAGS)
fe_launch_usrpayload_test_LDFLAGS = -L$(top_srcdir)/@LMONAPILOC@ 
fe_launch_usrpayload_test_LDADD   = -lmonfeapi 

#fe_multisession_launch_SOURCES    = fe_multisession_launch.cxx util.c
#fe_multisession_launch_LDFLAGS    = -L$(top_srcdir)/@LMONAPILOC@ 
#fe_multisession_launch_LDADD      = -lmonfeapi 

fe_attach_smoketest_SOURCES       = fe_attach_smoketest.cxx util.c
fe_attach_smoketest_CFLAGS        = @BUILD_FLAGS@ $(AM_CFLAGS)
fe_attach_smoketest_CXXFLAGS      = @BUILD_FLAGS@ $(AM_CXXFLAGS)
fe_attach_smoketest_LDFLAGS       = -L$(top_srcdir)/@LMONAPILOC@
fe_attach_smoketest_LDADD         = -lmonfeapi

be_kicker_SOURCES                 = be_kicker.cxx util.c
be_kicker_CFLAGS                  = @BUILD_FLAGS@ $(AM_CFLAGS)
be_kicker_CXXFLAGS                = @BUILD_FLAGS@ $(AM_CXXFLAGS)
be_kicker_LDFLAGS    	          = -L$(top_srcdir)/@LMONAPILOC@ 
be_kicker_LDADD                   = -lmonbeapi 

be_kicker_usrpayload_test_SOURCES = be_kicker_usrpayload_test.cxx
be_kicker_usrpayload_test_CFLAGS   = @BUILD_FLAGS@ $(AM_CFLAGS)
be_kicker_usrpayload_test_CXXFLAGS = @BUILD_FLAGS@ $(AM_CXXFLAGS)
be_kicker_usrpayload_test_LDFLAGS = -L$(top_srcdir)/@LMONAPILOC@ 
be_kicker_usrpayload_test_LDADD   = -lmonbeapi

be_standalone_kicker_SOURCES      = be_standalone_kicker.cxx
be_standalone_kicker_CFLAGS       = @BUILD_FLAGS@ $(AM_CFLAGS)
be_standalone_kicker_CXXFLAGS     = @BUILD_FLAGS@ $(AM_CXXFLAGS)
be_standalone_kicker_LDFLAGS      = -L$(top_srcdir)/@LMONAPILOC@ 
be_standalone_kicker_LDADD        = -lmonbeapi

fe_jobsnap_SOURCES                = fe_jobsnap.cxx util.c
fe_jobsnap_CFLAGS                 = @BUILD_FLAGS@ $(AM_CFLAGS)
fe_jobsnap_CXXFLAGS               = @BUILD_FLAGS@ $(AM_CXXFLAGS)
fe_jobsnap_LDFLAGS                = -L$(top_srcdir)/@LMONAPILOC@
fe_jobsnap_LDADD                  = -lmonfeapi 
be_jobsnap_SOURCES                = be_jobsnap.cxx util.c
be_jobsnap_CFLAGS                 = @BUILD_FLAGS@ $(AM_CFLAGS)
be_jobsnap_CXXFLAGS               = @BUILD_FLAGS@ $(AM_CXXFLAGS)
be_jobsnap_LDFLAGS                = -L$(top_srcdir)/@LMONAPILOC@
be_jobsnap_LDADD                  = -lmonbeapi

bin_SCRIPTS                       = test.LE_model_checker_mainthread \
				    test.LE_model_checker_pthread \
			            test.LE_model_checker_large \
				    test.LE_model_checker_extreme \
				    test.launch_1 \
				    test.launch_1_remote \
			   	    test.launch_2_half \
				    test.launch_2_uneven \
				    test.launch_3_invalid_dmonpath \
				    test.launch_5_usrpayload \
				    test.attach_1 \
				    test.attach_1_remote \
				    test.attach_1_pdebugmax \
				    test.attach_2_uneven \
				    test.attach_3_invalidpid \
				    test.attach_3_invalidpid_remote \
				    test.attach_3_callsafterfail \
				    test.attach_4_kill \
				    test.attach_4_shutdownbe \
				    test.jobsnap_1 \
				    test.launch_6_engine_failure \
				    test.fe_regErrorCB \
				    test.fe_regStatusCB


CLEANFILES                        = $(bin_SCRIPTS)
EXTRA_DIST                        = test.LE_model_checker_mainthread.in \
                                    test.LE_model_checker_pthread.in \
                                    test.LE_model_checker_large.in \
                                    test.LE_model_checker_extreme.in \
                                    test.launch_1.in \
                                    test.launch_1_remote.in \
                                    test.launch_2_half.in \
                                    test.launch_2_uneven.in \
                                    test.launch_3_invalid_dmonpath.in \
                                    test.launch_5_usrpayload.in \
                                    test.attach_1.in \
                                    test.attach_1_remote.in \
                                    test.attach_1_pdebugmax.in \
                                    test.attach_2_uneven.in \
                                    test.attach_3_invalidpid.in \
                                    test.attach_3_invalidpid_remote.in \
                                    test.attach_3_callsafterfail.in \
                                    test.attach_4_kill.in \
                                    test.attach_4_shutdownbe.in \
                                    test.jobsnap_1.in \
				    test.launch_6_engine_failure.in \
				    test.fe_regErrorCB.in \
				    test.fe_regStatusCB.in


do_subst = sed -e 's,@LMON@,$(prefix)/bin/launchmon$(EXEEXT),g' \
               -e 's,@TJLPATH@,$(TARGET_JOB_LAUNCHER_PATH),g' \
               -e 's,@RMTP@,$(RM_TYPE),g' \
	       -e 's,@SMP@,$(SMPFACTOR),g' \
	       -e 's,@NNODES@,$(TESTNNODES),g' \
               -e 's,@EXE@,$(EXEEXT),g' 

test.LE_model_checker_mainthread: test.LE_model_checker_mainthread.in Makefile
	$(do_subst) < test.LE_model_checker_mainthread.in > test.LE_model_checker_mainthread
	chmod +x test.LE_model_checker_mainthread
test.LE_model_checker_pthread: test.LE_model_checker_pthread.in Makefile
	$(do_subst) < test.LE_model_checker_pthread.in > test.LE_model_checker_pthread
	chmod +x test.LE_model_checker_pthread
test.LE_model_checker_large: test.LE_model_checker_large.in Makefile
	$(do_subst) < test.LE_model_checker_large.in > test.LE_model_checker_large
	chmod +x test.LE_model_checker_large
test.LE_model_checker_extreme: test.LE_model_checker_extreme.in Makefile
	$(do_subst) < test.LE_model_checker_extreme.in > test.LE_model_checker_extreme
	chmod +x test.LE_model_checker_extreme
test.launch_1: test.launch_1.in Makefile
	$(do_subst) < test.launch_1.in > test.launch_1
	chmod +x test.launch_1
test.launch_1_remote: test.launch_1_remote.in Makefile
	$(do_subst) < test.launch_1_remote.in > test.launch_1_remote
	chmod +x test.launch_1_remote
test.launch_2_half: test.launch_2_half.in Makefile
	$(do_subst) < test.launch_2_half.in > test.launch_2_half
	chmod +x test.launch_2_half 
test.launch_3_invalid_dmonpath: test.launch_3_invalid_dmonpath.in Makefile
	$(do_subst) < test.launch_3_invalid_dmonpath.in > test.launch_3_invalid_dmonpath
	chmod +x test.launch_3_invalid_dmonpath
test.launch_2_uneven: test.launch_2_uneven.in Makefile
	$(do_subst) < test.launch_2_uneven.in > test.launch_2_uneven
	chmod +x test.launch_2_uneven
test.launch_5_usrpayload: test.launch_5_usrpayload.in Makefile
	$(do_subst) < test.launch_5_usrpayload.in > test.launch_5_usrpayload
	chmod +x test.launch_5_usrpayload 
test.launch_6_engine_failure: test.launch_6_engine_failure.in Makefile
	$(do_subst) < test.launch_6_engine_failure.in > test.launch_6_engine_failure
	chmod +x test.launch_6_engine_failure
test.attach_1: test.attach_1.in Makefile
	$(do_subst) < test.attach_1.in > test.attach_1
	chmod +x test.attach_1
test.attach_1_pdebugmax: test.attach_1_pdebugmax.in Makefile
	$(do_subst) < test.attach_1_pdebugmax.in > test.attach_1_pdebugmax
	chmod +x test.attach_1_pdebugmax
test.attach_1_remote: test.attach_1_remote.in Makefile
	$(do_subst) < test.attach_1_remote.in > test.attach_1_remote
	chmod +x test.attach_1_remote
test.attach_2_uneven: test.attach_2_uneven.in Makefile
	$(do_subst) < test.attach_2_uneven.in > test.attach_2_uneven
	chmod +x test.attach_2_uneven
test.attach_3_invalidpid: test.attach_3_invalidpid.in Makefile
	$(do_subst) < test.attach_3_invalidpid.in > test.attach_3_invalidpid
	chmod +x test.attach_3_invalidpid
test.attach_3_invalidpid_remote: test.attach_3_invalidpid_remote.in Makefile
	$(do_subst) < test.attach_3_invalidpid_remote.in > test.attach_3_invalidpid_remote
	chmod +x test.attach_3_invalidpid_remote
test.attach_3_callsafterfail: test.attach_3_callsafterfail.in Makefile
	$(do_subst) < test.attach_3_callsafterfail.in > test.attach_3_callsafterfail
	chmod +x test.attach_3_callsafterfail
test.attach_4_kill: test.attach_4_kill.in Makefile
	$(do_subst) < test.attach_4_kill.in > test.attach_4_kill
	chmod +x test.attach_4_kill
test.attach_4_shutdownbe: test.attach_4_shutdownbe.in Makefile
	$(do_subst) < test.attach_4_shutdownbe.in > test.attach_4_shutdownbe
	chmod +x test.attach_4_shutdownbe
test.jobsnap_1: test.jobsnap_1.in Makefile
	$(do_subst) < test.jobsnap_1.in > test.jobsnap_1
	chmod +x test.jobsnap_1
test.fe_regErrorCB: test.fe_regErrorCB.in Makefile
	$(do_subst) < test.fe_regErrorCB.in > test.fe_regErrorCB
	chmod +x test.fe_regErrorCB
test.fe_regStatusCB: test.fe_regStatusCB.in Makefile
	$(do_subst) < test.fe_regStatusCB.in > test.fe_regStatusCB
	chmod +x test.fe_regStatusCB

run_3mins: run_3mins.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) @BUILD_FLAGS@ -I$(top_srcdir)/launchmon/src $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	
hang_on_SIGUSR1: hang_on_SIGUSR1.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) @BUILD_FLAGS@ -I$(top_srcdir)/launchmon/src $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	
simple_MPI: simple_MPI.c Makefile
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) @BUILD_FLAGS@ -I$(top_srcdir)/launchmon/src $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -o $@ $<	
#LMBench: LMBench.cxx Makefile
#	/bin/sh $(top_srcdir)/libtool --tag=CXX --mode=link $(MPICXX) $(DEFS) $(DEFAULT_INCLUDES) -I$(top_srcdir)/launchmon/src -I$(top_srcdir)/@LMONAPILOC@ $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) -o $@ $< $(top_srcdir)/@LMONAPILOC@/lmon_be_comm.o  $(top_srcdir)/@LMONAPILOC@/lmon_say_msg.o -L$(top_srcdir)/@LMONAPILOC@ -lmonbeapi 

