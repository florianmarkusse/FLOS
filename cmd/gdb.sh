#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../

SERIAL_FOLDER="no-serial"

function display_usage() {
    echo -e "Usage: $0 [${YELLOW}OPTIONS${NO_COLOR}]"
    echo -e "Script to attach gdb to QEMU debug build."
    echo -e "${BOLD}Options:${NO_COLOR}"
    echo -e "  -s, --serial               Use the build with output to serial ."
    echo -e "  -h, --help                 Display this help message."
    exit 1
}


while [[ "$#" -gt 0 ]]; do
    case $1 in
    -h | --help)
        display_usage
        ;;
    -s | --serial)
        SERIAL_FOLDER="serial"
        shift 1;
        ;;
    *)
        display_usage
        ;;
    esac
done


FILE_REGEX="./projects/kernel/code/build/.*/${SERIAL_FOLDER}/DEBUG/kernel"
gdb -ex "target remote localhost:1234" \
    -ex "dashboard -layout source stack threads assembly variables breakpoints memory history !registers !expressions" \
    -ex "file $(find . -type f -regex "${FILE_REGEX}")"
