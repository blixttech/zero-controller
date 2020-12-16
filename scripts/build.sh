#!/bin/bash

script_name_full=$(basename "${0}")
script_name_base="${script_name_full%%.*}"
script_file_full=$(readlink -f "${0}")
script_file_base="${script_file_full%%.*}"
script_dir=$(dirname "${script_file_full}")
script_dir_full=$(readlink -f "${script_dir}")
config_file_full="${script_file_base}.conf"

project_dir=$(dirname $(readlink -f "${script_dir_full}"))

output=$(grep docker /proc/1/cgroup)
if [ -z "${output}" ]; then
    in_docker=0
else
    in_docker=1
fi

if [ "${in_docker}" -eq 1 ]; then
    build_dir="${project_dir}/build-docker"
    sudo pip install conan --upgrade
else
    build_dir="${project_dir}/build"
    pip install conan --upgrade
fi

conan remote add bincreators https://api.bintray.com/conan/bincreators/public-conan
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

if [ ! -d "${build_dir}" ]; then
    mkdir "${build_dir}"
fi

conan create "${project_dir}" demo/testing
conan install --install-folder "${build_dir}" bk710-tool/master@demo/testing