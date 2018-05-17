#!/bin/bash

# This script builds the iOS and Mac snappy libraries.
# Automake will be used to build snappy into static library.
# Reference: http://blog.csdn.net/stark_summer/article/details/47360567
# How to check the deployment target of a static library:
# otool -lv snappy/lib/iOS/libsnappy.a | grep version

set -e

SNAPPY_VERSION="snappy-1.1.3"
DEVELOPER=`xcode-select -print-path`
DEFAULT_IOSVERSION=`defaults read $(xcode-select -print-path)/Platforms/iPhoneOS.platform/version CFBundleShortVersionString`
MACOSX_DEPLOYMENT_TARGET=10.4  # -mmacosx-version-min
IPHONEOS_DEPLOYMENT_TARGET=7.0 # -miphoneos-version-min/-mios-simulator-version-min

usage ()
{
	echo "usage: $0 [minimum iOS SDK version (default ${DEFAULT_IOSVERSION})]"
	exit 127
}

if [ "$1"x == "-h"x ]; then
	usage
fi

SDK_VERSION="${DEFAULT_IOSVERSION}"
if [ ! -z $1 ]; then
	IPHONEOS_DEPLOYMENT_TARGET=$1
fi

buildMac()
{
	ARCH=$1

	echo "Building ${SNAPPY_VERSION} for ${ARCH}"

	pushd . > /dev/null
	cd "${SNAPPY_VERSION}"
	./configure --prefix="/tmp/${SNAPPY_VERSION}-${ARCH}" \
				CC="${DEVELOPER}/usr/bin/gcc -arch ${ARCH}" \
				CXX="${DEVELOPER}/usr/bin/g++ -arch ${ARCH}" \
				CPP="${DEVELOPER}/usr/bin/gcc -E" \
				CXXPP="${DEVELOPER}/usr/bin/g++ -E" &> "/tmp/${SNAPPY_VERSION}-${ARCH}.log" 2>&1
	make >> "/tmp/${SNAPPY_VERSION}-${ARCH}.log" 2>&1
	make install >> "/tmp/${SNAPPY_VERSION}-${ARCH}.log" 2>&1
	make clean >> "/tmp/${SNAPPY_VERSION}-${ARCH}.log" 2>&1
	popd > /dev/null
}

buildIOS()
{
	ARCH=$1
  
	if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]]; then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi

	CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
	CROSS_SDK="${PLATFORM}${SDK_VERSION}.sdk"
	SDK="${CROSS_TOP}/SDKs/${CROSS_SDK}"
	CROSSFLAGS="-isysroot ${SDK} -miphoneos-version-min=${IPHONEOS_DEPLOYMENT_TARGET}"

	echo "Building ${SNAPPY_VERSION} for ${PLATFORM} ${IPHONEOS_DEPLOYMENT_TARGET} ${ARCH}"

	pushd . > /dev/null
	cd "${SNAPPY_VERSION}"
	./configure --prefix="/tmp/${SNAPPY_VERSION}-iOS-${ARCH}" --enable-cross-compile --host=arm-apple-darwin\
				CC="${DEVELOPER}/usr/bin/gcc ${CROSSFLAGS} -arch ${ARCH}" \
				CXX="${DEVELOPER}/usr/bin/g++ ${CROSSFLAGS} -arch ${ARCH}" \
				CPP="${DEVELOPER}/usr/bin/gcc -E" \
				CXXPP="${DEVELOPER}/usr/bin/g++ -E" &> "/tmp/${SNAPPY_VERSION}-iOS-${ARCH}.log" 2>&1
	make >> "/tmp/${SNAPPY_VERSION}-iOS-${ARCH}.log" 2>&1
	make install >> "/tmp/${SNAPPY_VERSION}-iOS-${ARCH}.log" 2>&1
	make clean >> "/tmp/${SNAPPY_VERSION}-iOS-${ARCH}.log" 2>&1
	popd > /dev/null
}

echo "Cleaning up"
rm -rf snappy/*

mkdir -p snappy/lib/iOS
mkdir -p snappy/lib/Mac
mkdir -p snappy/include/

rm -rf "/tmp/${SNAPPY_VERSION}-*"
rm -rf "/tmp/${SNAPPY_VERSION}-*.log"

rm -rf "${SNAPPY_VERSION}"

if [ ! -e ${SNAPPY_VERSION}.tar.gz ]; then
	echo "Downloading ${SNAPPY_VERSION}.tar.gz"
	# https://codeload.github.com/google/snappy/tar.gz/1.1.3
	# https://codeload.github.com/google/snappy/legacy.tar.gz/1.1.3
	target_version=`echo ${SNAPPY_VERSION} | sed 's/snappy-//g'`
	curl -o ${SNAPPY_VERSION}.tar.gz "https://codeload.github.com/google/snappy/tar.gz/${target_version}"
else
	echo "Using ${SNAPPY_VERSION}.tar.gz"
fi

echo "Unpacking snappy"
# unzip "${SNAPPY_VERSION}.zip" > /dev/null
tar xfz "${SNAPPY_VERSION}.tar.gz"

pushd . > /dev/null
cd ${SNAPPY_VERSION}
# Fix the problem in `autogen.sh`.
sed -ie 's/^libtoolize/glibtoolize/' autogen.sh
./autogen.sh >> "/tmp/${SNAPPY_VERSION}-autogen.log" 2>&1
popd > /dev/null

buildMac "i386"
buildMac "x86_64"

echo "Copying headers"
cp /tmp/${SNAPPY_VERSION}-i386/include/* snappy/include/

echo "Building Mac libraries"
lipo \
	"/tmp/${SNAPPY_VERSION}-i386/lib/libsnappy.a" \
	"/tmp/${SNAPPY_VERSION}-x86_64/lib/libsnappy.a" \
	-create -output snappy/lib/Mac/libsnappy.a

buildIOS "armv7"
buildIOS "armv7s"
buildIOS "arm64"
buildIOS "x86_64"
buildIOS "i386"

echo "Building iOS libraries"
lipo \
	"/tmp/${SNAPPY_VERSION}-iOS-armv7/lib/libsnappy.a" \
	"/tmp/${SNAPPY_VERSION}-iOS-armv7s/lib/libsnappy.a" \
	"/tmp/${SNAPPY_VERSION}-iOS-arm64/lib/libsnappy.a" \
	"/tmp/${SNAPPY_VERSION}-iOS-i386/lib/libsnappy.a" \
	"/tmp/${SNAPPY_VERSION}-iOS-x86_64/lib/libsnappy.a" \
	-create -output snappy/lib/iOS/libsnappy.a

echo "Cleaning up"
rm -rf /tmp/${SNAPPY_VERSION}-*
rm -rf ${SNAPPY_VERSION}

echo "Done"
