#!/bin/bash

FLUX_HELPER_JOBID='%^+_no'
declare -r long_opts='help,jobid:'
declare -r short_opts='hJ:'
declare -r prog=${0##*/}
declare -r usage="
Usage: ${prog} [OPTIONS] -- tool daemon launch string\n\
\n\
Spawn the tool daemon specified in the launch tool string\n\
on the nodes where the Flux job is running.\n\
Launch one daemon process per node.\n\
\n\
Options:\n\
 -h, --help                    Display this message\n\
 -J, --jobid=JOBID             Jobid of the target Flux job\n\
     --                        Stop parsing options after this\n\
\n\
"

die() { echo -e "${prog}:" "$@"; exit 1; }

GETOPTS=$(/usr/bin/getopt -o ${short_opts} -l ${long_opts} -n "${prog}" -- "${@}")
eval set -- "${GETOPTS}"

while true; do
    case "${1}" in
      -h|--help)                   echo -ne "${usage}";          exit 0  ;;
      -J|--jobid)                  FLUX_HELPER_JOBID="${2}";     shift 2 ;;
      --)                          shift; break;                         ;;
      *)                           die "Invalid option '${1}'\n${usage}" ;;
    esac
done

flux -h > /dev/null 2>&1 || die "flux is not in your path"

[[ ${FLUX_HELPER_JOBID} = '%^+_no' ]] && die "--jobid=JOBID is required"

[[ $# -lt 1 ]] && die "daemon launch string is required"

FLUX_RANK_SET=$(flux jobs -no {ranks} ${FLUX_HELPER_JOBID})

flux exec -r ${FLUX_RANK_SET} "${@}"

#
# vi:tabstop=4 shiftwidth=4 expandtab
#
