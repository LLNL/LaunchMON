[![Build Status](https://travis-ci.org/LLNL/LaunchMON.svg?branch=master)](https://travis-ci.org/LLNL/LaunchMON)

## LaunchMON

LaunchMON is a software infrastructure that enables HPC run-time tools
to co-locate tool daemons with a parallel job. Its API allows a tool to
identify all the remote processes of a job and to scalably launch daemons
into the relevant nodes.

#### Building LaunchMON

LaunchMON requires the following packages to build

```
libelf
boost
boost-devel
libgcrypt >= 1.4.5
libgcrypt-devel >= 1.4.5
libgpg-error >= 1.7.0
# for secure handshake
munge
```

##### Building from Source

The following is the simplest way to configure, build and install
the LaunchMON package from the source checked out from this repo.

If you are building from a distribution, skip to step (2).

(1) `cd` into the root directory of this package

(2) Bootstrap the package:

   `% ./bootstrap`

(3) Configure:

   `% ./configure --prefix=/foo/tool/lmon --with-test-rm=slurm`

   For example, the above configuration
   command will configure the package. The optional --prefix
   switch will set /foo/tool/lmon as the prefix directory
   for installation; the optional --with-test-rm will
   configure test cases to be built for SLURM resource
   manager (RM).

   `./configure --help` prints other available configuration
   options.

(4) Build:

   `% make`

   will build the package directly within the source tree.

(5) Install:

   `% make install`

(6) Smoke test:

   `% make check`

   will build the test codes in the test/src directory within the source tree.
   `cd test/src` directory under the source directory
   and run `test.launch_1` and `test.attach_1` as your smoke tests.
   Note that these test cases assume that you alreay have an
   allocation (or interactive partition) under the invoking
   shell so that RM job launcher (e.g. srun) can launch
   and execute a parallel target application. Also ensure that '.' is in
   your $PATH when you run the smoke test scripts.

##### Building with Spack

LaunchMON and its dependencies are also packaged via the
[Spack](https://spack.readthedocs.io) package manager. It can be installed
with the following commands:

```
git clone https://github.com/spack/spack.git
cd spack
./bin/spack install launchmon
```
