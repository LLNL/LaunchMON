## $Header: $
##
## Makefile.am -- Process this file with automake to produce Makefile.in 
##
##--------------------------------------------------------------------------------
## Copyright (c) 2013, Lawrence Livermore National Security, LLC. Produced at
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
##  This is a RMP spec file. After svn-export the source tree,
##  ./configure followed by make dist will create a source distribution.
##  That distribution along with launchmon.spec and launchmon.module
##  should suffice in creating RPM files. 
##
##  Update Log:
##        Jan 11 2013 DHA: Created a file
##

Name:           launchmon
Version:        1.0.beta1 
Release:        1cce
License:        LGPL
Group:          Development/Languages
Summary:        Tool Daemon launching/bootstrapping infrastructure. 
URL:            https://sourceforge.net/projects/launchmon

# to set up an rpm to install to /opt, adjust install_prefix and module_path
# accordingly

%define install_prefix /g/g0/dahn/rpm-local/opt/%{name}-%{version}
%define module_path    /g/g0/dahn/rpm-local/opt/modules/modulefiles/launchmon


SOURCE: 	%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# Build time dependencies:
BuildRequires:  boost-devel
BuildRequires:  elfutils-libelf-devel
BuildRequires:  slurm-devel 
BuildRequires:  mvapich-gnu

# A line shouldn't be longer than 80 characters.
%description
LaunchMON is scalable software infrastructure that enables an HPC run-time tool 
to co-locate its daemons with the target parallel application. Its API allows 
the tool to identify all the remote processes of the application and to launch 
its daemons into the relevant remote nodes.

More information can be found at https://sourceforge.net/projects/launchmon.

%prep
%setup -q -n %{name}-%{version}
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

%build
. /usr/share/[mM]odules/init/bash
#module load mvapich-gnu-shmem/1.2

./configure \
    --prefix=%{install_prefix} \
    --with-test-rm=slurm \
    --with-test-nnodes=2 \
    --with-test-ncore-per-CN=8 \
    --with-mw-hostlist=sierra1620:sierra1626

make 

%install
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p ${RPM_BUILD_ROOT}%{module_path}
cp ${RPM_SOURCE_DIR}/launchmon.module ${RPM_BUILD_ROOT}%{module_path}/%{version}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{install_prefix}
%attr(644, root, root) %{module_path}/%{version}

