#!/bin/bash

script_name_full=$(basename "${0}")
script_name_base="${script_name_full%%.*}"
script_file_full=$(readlink -f "${0}")
script_file_base="${script_file_full%%.*}"
script_dir=$(dirname "${script_file_full}")
script_dir_full=$(readlink -f "${script_dir}")
config_file_full="${script_file_base}.conf"

project_dir=$(dirname $(readlink -f "${script_dir_full}"))

echo "Starting docker"
docker run -it -v"${project_dir}":/home/conan/project --rm conanio/gcc5 /home/conan/project/scripts/build.sh