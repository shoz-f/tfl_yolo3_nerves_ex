#!/bin/bash 

set -x
set -e

case "${MIX_TARGET}" in
    "rpi"|"rpi0")
        PATH=~/.nerves/artifacts/nerves_toolchain_armv6_rpi_linux_gnueabi-linux_x86_64-1.3.2/bin:$PATH
	;;

    "rpi2"|"rpi3")
        PATH=~/.nerves/artifacts/nerves_toolchain_arm_unknown_linux_gnueabihf-linux_x86_64-1.3.2/bin:$PATH
	;;

    *) echo "Unknown target: ${MIX_TARGET}"
       exit 1
       ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TENSORFLOW_DIR="${SCRIPT_DIR}/../../../.."

FREE_MEM="$(free -m | awk '/^Mem/ {print $2}')"
# Use "-j 4" only memory is larger than 2GB
if [[ "FREE_MEM" -gt "2000" ]]; then
  NO_JOB=4
else
  NO_JOB=1
fi

make -j ${NO_JOB} TARGET=nerves -C "${TENSORFLOW_DIR}" -f tensorflow/lite/tools/make/Makefile $@
