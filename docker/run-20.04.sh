#!/usr/bin/env bash

set -u
set -e

UTIL_PATH=$(readlink -f "$0")
UTIL_NAME=$(basename "$UTIL_PATH")
UTIL_DIR=$(dirname "$UTIL_PATH")

DOCKER_IMAGE="trustbox-oe-builder-20.04"
HOST=$(hostname)

err() {
	echo "Error: $@" >& 2
	exit 1
}

usage() {
    cat << EOF
Usage: $UTIL_NAME
Run docker docker environment.

Arguments:
  -h, --help    display this help and exit
EOF
}

parse_arguments() {
	options=$(getopt -o h --long help: -- "$@")
	[ $? -eq 0 ] || {
		usage
		exit 1
	}
	eval set -- "$options"
	while true; do
		case "$1" in
		-h|--help) usage ; exit ;;
		--) shift ; break ;;
		esac
		shift
	done

	WORK_DIR=$(readlink -f "$UTIL_DIR"/..)
	DOCKER_DIR=$(readlink -f "$WORK_DIR/docker")
}

docker_build() {
	# Check if image already exists
	if docker image inspect "$DOCKER_IMAGE" &> /dev/null; then
		return
	else
		make
	fi
}

docker_run() {
	docker run --rm -ti -h "$HOST" --net=host -v $WORK_DIR:$WORK_DIR -v $HOME/.bash_history:$HOME/.bash_history -w $WORK_DIR --add-host "$HOST:127.0.0.1" "$DOCKER_IMAGE" /bin/bash
}


parse_arguments $@

cd "$DOCKER_DIR"

docker_build
docker_run
