#!/bin/bash

script_name_full=$(basename "${0}")
script_name_base="${script_name_full%%.*}"
script_file_full=$(readlink -f "${0}")
script_file_base="${script_file_full%%.*}"
script_dir=$(dirname "${script_file_full}")
script_dir_full=$(readlink -f "${script_dir}")
config_file_full="${script_file_base}.conf"

script_usage="Usage: ${script_name_full} [OPTIONS]

Options:
  -b, --build       Build            
  -p, --package     Package
  -h, --help        Show this help
"
do_build=0
do_package=0

for key in "${@}"; do
    case "${key}" in
        -b|--build)
            do_build=1
        ;;
        -p|--package)
            do_package=1
        ;;
        -h|--help)
            echo "${script_usage}"
            exit 0
        ;;
        *)
            echo "ERROR: Unknown option '${key}'"
            echo "${script_usage}"
            exit 1
        ;;
    esac
done

project_dir=$(dirname $(readlink -f "${script_dir_full}"))

output=$(cd ${project_dir} && git rev-parse --is-inside-work-tree 2> /dev/null)
if [ -z "${output}" ]; then
    echo "${project_dir} is not a valid source repository."
    exit 1
fi

#output=$(cd ${project_dir} && git describe --all | sed 's/^.*\/v\?//g')
#if [ -z "${output}" ]; then
#    echo "Invalid git ref (branch or tag): '${output}'."
#    exit 1
#fi
#conan_version=${output}

output=$(grep docker /proc/1/cgroup)
if [ -z "${output}" ]; then
    in_docker=0
else
    in_docker=1
fi

if [ "${in_docker}" -eq 1 ]; then
    build_dir="${project_dir}/build-local/build-docker"
    package_dir="${project_dir}/build-local/package-docker"

    sudo pip install pip --upgrade
    sudo pip install conan
    conan remote add blixttech-bintray https://api.bintray.com/conan/blixttech/conan-packages
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
else
    build_dir="${project_dir}/build-local/build"
    package_dir="${project_dir}/build-local/package"
fi

export CONAN_SYSREQUIRES_MODE="enabled"

if [ "${do_build}" -eq 1 ]; then
    if [ ! -d "${build_dir}" ]; then
        mkdir -p "${build_dir}"
    fi

    conan install "${project_dir}" --install-folder "${build_dir}"
    conan build "${project_dir}" --build-folder "${build_dir}"

    if [ "${do_package}" -eq 1 ]; then
        if [ ! -d "${package_dir}" ]; then
            mkdir -p "${package_dir}"
        fi

        conan package "${project_dir}" --build-folder "${build_dir}" --package-folder "${package_dir}"
    fi
fi
