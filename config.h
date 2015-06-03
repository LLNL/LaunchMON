/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define 1 for AIX_CODE_REQUIRED */
/* #undef AIX_CODE_REQUIRED */

/* 64bit */
#define BIT64 1

/* Define 1 for COBO_BASED */
#define COBO_BASED 1

/* Define a beginning port for COBO_BASED */
#define COBO_BEGIN_PORT 20101

/* Define 32 for COBO_BASED */
#define COBO_PORT_RANGE 32

/* Define DEBUG flag */
#define DEBUG 1

/* Allow NULL encryption */
/* #undef ENABLE_NULL_ENCRYPTION */

/* env command found */
#define ENVCMD "/usr/bin/env"

/* 1 if defined */
#define GCRYPT 1

/* Define to 1 if you have the <algorithm> header file. */
#define HAVE_ALGORITHM 1

/* Define to 1 if you have the <cxxabi.h> header file. */
#define HAVE_CXXABI_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the <fstream> header file. */
#define HAVE_FSTREAM 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <iostream> header file. */
#define HAVE_IOSTREAM 1

/* Define 1 if we have libelf/libelf.h */
#define HAVE_LIBELF_H 1

/* Define to 1 if you have the <libelf/libelf.h> header file. */
/* #undef HAVE_LIBELF_LIBELF_H */

/* Define to 1 if you have the `munge' library (-lmunge). */
#define HAVE_LIBMUNGE 1

/* Define to 1 if you have the <list> header file. */
#define HAVE_LIST 1

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <map> header file. */
#define HAVE_MAP 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define to 1 if you have the <stack> header file. */
#define HAVE_STACK 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <string> header file. */
#define HAVE_STRING 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <vector> header file. */
#define HAVE_VECTOR 1

/* Define to 1 if you have the `vfork' function. */
#define HAVE_VFORK 1

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if `fork' works. */
#define HAVE_WORKING_FORK 1

/* Define to 1 if `vfork' works. */
#define HAVE_WORKING_VFORK 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Use keyfile for authentication */
/* #undef KEYFILE */

/* Define 1 for LINUX_CODE_REQUIRED */
#define LINUX_CODE_REQUIRED 1

/* install prefix */
#define LMON_PREFIX "/g/g19/liu50/bin/cab"

/* Define to header that first defines elf. */
#define LOCATION_OF_LIBELFHEADER <libelf.h>

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define 1 for MEASURE_TRACING_COST */
/* #undef MEASURE_TRACING_COST */

/* Define 1 for MPI_BASED */
/* #undef MPI_BASED */

/* Use munge for authentication */
#define MUNGE 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "launchmon"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "ahn1@llnl.gov"

/* Define to the full name of this package. */
#define PACKAGE_NAME "LaunchMON"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "LaunchMON 1.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "launchmon"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.1"

/* Define 1 for PPC_ARCHITECTURE */
/* #undef PPC_ARCHITECTURE */

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* be starter stub location */
/* #undef RM_BE_STUB_CMD */

/* bulk launcher location */
/* #undef RM_FE_COLOC_CMD */

/* rsh or its equivalent found */
#define RSHCMD "/usr/bin/rsh"

/* Directory to store key files in */
#define SEC_KEYDIR ""

/* ssh or its equivalent found */
#define SSHCMD "/usr/bin/ssh"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define 1 for SUB_ARCH_ALPS */
/* #undef SUB_ARCH_ALPS */

/* Define 1 for SUB_ARCH_BGL */
/* #undef SUB_ARCH_BGL */

/* Define 1 for SUB_ARCH_BGP */
/* #undef SUB_ARCH_BGP */

/* Define 1 for SUB_ARCH_BGQ */
/* #undef SUB_ARCH_BGQ */

/* Define os-isa string */
#define TARGET_OS_ISA_STRING "linux-x86_64"

/* Define rm string */
#define TARGET_RM_STRING "generic"

/* Define test more coll support */
#define TEST_MORE_COLL 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define TOOL_HOST_ENV */
#define TOOL_HOST_ENV "LMON_FE_WHERETOCONNECT_ADDR"

/* Define TOOL_PORT_ENV */
#define TOOL_PORT_ENV "LMON_FE_WHERETOCONNECT_PORT"

/* Define TOOL_SCH_ENV */
#define TOOL_SCH_ENV "LMON_SEC_CHK"

/* Define TOOL_SS_ENV */
#define TOOL_SS_ENV "LMON_SHARED_SECRET"

/* totalview found */
#define TVCMD "/usr/local/bin/totalview"

/* Define USE_VERBOSE_LOGDIR flag */
/* #undef USE_VERBOSE_LOGDIR */

/* Define VERBOSE flag */
/* #undef VERBOSE */

/* Define LOGDIR */
/* #undef VERBOSE_LOGDIR */

/* Version number of package */
#define VERSION "1.0.1"

/* Define 1 for X86_64_ARCHITECTURE */
#define X86_64_ARCHITECTURE 1

/* Define 1 for X86_ARCHITECTURE */
/* #undef X86_ARCHITECTURE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */
