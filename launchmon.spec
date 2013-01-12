Name:           launchmon
Version:        1.0.beta1 
Release:        1cce
License:        LGPL
Group:          Development/Languages
Summary:        Tool Daemon launching/bootstrapping infrastructure. 
URL:            https://sourceforge.net/projects/launchmon
# to specify that this package can be relocated by striping
# the prefix from the locations
#Prefix:		/g/g/0/dahn/rpm-local/opt
# to set up an rpm to install to /opt, adjust install_prefix and module_path
# accordingly

%define install_prefix /g/g0/dahn/rpm-local/opt/%{name}-%{version}
%define module_path    /g/g0/dahn/rpm-local/opt/modules/modulefiles/launchmon
%define debug_package %{nil}

Source0: 	%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# Build time dependencies:
BuildRequires:  gcc
BuildRequires:  libgcc
BuildRequires:  compat-libstdc++
BuildRequires:  glibc
BuildRequires:  perl
BuildRequires:  boost-devel
BuildRequires:  elfutils-libelf-devel
#
# Configuring test cases with slurm and mvapich
BuildRequires:  slurm-devel 
BuildRequires:  mvapich-gnu

# In case you need to configure test cases with
# openmpi instead of slurm
# BuildRequires: openmpi-1.6-gnu

#
# Install time dependencies
Requires:  bash
Requires:  mvapich-gnu
Requires:  glibc
Requires:  elfutils-libelf-devel



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
module load mvapich-gnu-shmem/1.2

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




