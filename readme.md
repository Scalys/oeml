# OpenEnclave Machine Learning demo

This demo performs a confidential machine learning inference within OpenEnclave.
This project is a part of a  [Blueprint for a Confidential Computing at the Edge](https://github.com/Azure-Samples/Project_Confidential_Apps_for_IoT_with_Enclaves).

The main function of the application is to perform an image classification
between three companies logo images (Scalys, Arm and Microsoft). It can work
both in a single shot mode clasifying an image file and in a video stream capture mode.

# Hardware platform

Demo setup consists of [TrustBox Edge 201](https://scalys.com/trustbox-edge-201/) and an optional usb camera for trying
out streamed operation mode.

Demo is built and tested with web camera Logitech C270. Potentially any generic
USB camera should  work, but reliable functioning is only tested on C270. Other
cameras may experience worse classification quality, as the machine learning
model was trained on the images captured from C270.

# Build

Build is verified and tested on an Ubuntu 20.04 host system. For reprodution SDK with toolchain should be installed.

## Build environment

To have a reproducible environment docker build environment from this project
can be used. To prepare and run it perform:
```
$ cd docker
$ make
```

To run a session witihn this environment:
```
$ docker/run-20.04.sh
```

## Toolchain

A successful build of this project requires a specific SDK to be used. Download it from
Scalys releases [here](http://trustbox.scalys.com/pub/openenclave/poky-glibc-x86_64-scalys-openenclave-image-aarch64-trustsom-toolchain-ext-3.1.10.sh) and install it via command:

```
$ chmod +x ./poky-glibc-x86_64-scalys-openenclave-image-aarch64-trustsom-toolchain-ext-3.1.10.sh
$ MACHINE=trustsom ./poky-glibc-x86_64-scalys-openenclave-image-aarch64-trustsom-toolchain-ext-3.1.10.sh
```

~Note:~ docker build environment comes with this toolchain already downloaded and
pre-installed during build stage.

## Application bulid

Source the toolchain:
```
$ source ~/poky_sdk/environment-setup-aarch64-poky-linux
```

Create a build directory next to the downloaded oeml demo:
```
mkdir oeml-build
cd oeml-build
```

Configure and build the application:
```
cmake ../oeml -DPKG_CONFIG_SYSROOT_DIR="${PKG_CONFIG_SYSROOT_DIR}"  -DOE_TEE=OP-TEE -DOE_PACKAGE_OPTEE_PLATFORM=trustsom -DOE_OPTEE_PLATFORM=trustsom -DCMAKE_TOOLCHAIN_FILE=../oeml/cmake/arm-cross.cmake -DCMAKE_BUILD_TYPE=release -DOE_PACKAGE_PREFIX=${PKG_CONFIG_SYSROOT_DIR}/opt/oe
make
```

The resulting build outputs are located at:
```
out/bin/1f574668-6c89-41b5-b313-4b2d85d63c9d.ta
out/bin/oeml
```

# Run

To run the demo manually on a device in a single shot mode:
```
# ./oeml 1f574668-6c89-41b5-b313-4b2d85d63c9d.ta scalys.png
```
A successful execution should print `scalys`.

To run in a streamed video mode from camera /dev/video0 skip the argument:
```
# ./oeml 1f574668-6c89-41b5-b313-4b2d85d63c9d.ta
```
Successfull execution will produce a continuous stream of label names classified from the camera.

For more details see `./oeml --help`.
