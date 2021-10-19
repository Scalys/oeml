all:
	docker build -t trustbox-oe-builder-20.04 --network host --build-arg USER=$(USER) --build-arg UID=`id -u` --build-arg GID=`id -g` . -f Dockerfile-20.04
