proj_top=`pwd`/..

# setup deb packages
deb_repo=http://ftp.jp.debian.org/debian/pool/main

case "${MIX_TARGET}" in
    "rpi"|"rpi0")
        wget -nc ${deb_repo}/libj/libjpeg-turbo/libjpeg62-turbo-dev_1.5.1-2_armel.deb
        dpkg -x libjpeg62-turbo-dev_1.5.1-2_armel.deb .
    
        wget -nc ${deb_repo}/libj/libjpeg-turbo/libjpeg62-turbo_1.5.1-2_armel.deb
        dpkg -x libjpeg62-turbo_1.5.1-2_armel.deb .
	;;

    "rpi2"|"rpi3")
        wget -nc ${deb_repo}/libj/libjpeg-turbo/libjpeg62-turbo-dev_1.5.1-2_armhf.deb
        dpkg -x libjpeg62-turbo-dev_1.5.1-2_armhf.deb .
    
        wget -nc ${deb_repo}/libj/libjpeg-turbo/libjpeg62-turbo_1.5.1-2_armhf.deb
        dpkg -x libjpeg62-turbo_1.5.1-2_armhf.deb .
	;;

    *) echo "Unknown target: ${MIX_TARGET}"
       exit 1
       ;;
esac

wget -nc ${deb_repo}/c/cimg/cimg-dev_2.4.5+dfsg-1_all.deb
dpkg -x cimg-dev_2.4.5+dfsg-1_all.deb .

wget -nc ${deb_repo}/n/nlohmann-json3/nlohmann-json3-dev_3.5.0-0.1_all.deb
dpkg -x nlohmann-json3-dev_3.5.0-0.1_all.deb .

# copy shared lib to target rootfs_overlay
mkdir -p ${proj_top}/rootfs_overlay/usr/lib

rm ${proj_top}/rootfs_overlay/usr/lib/libjpeg.so*
case "${MIX_TARGET}" in
    "rpi"|"rpi0")
        cp -au usr/lib/arm-linux-gnueabi/libjpeg.so* ${proj_top}/rootfs_overlay/usr/lib
	;;

    "rpi2"|"rpi3")
        cp -au usr/lib/arm-linux-gnueabihf/libjpeg.so* ${proj_top}/rootfs_overlay/usr/lib
	;;
esac

# setup other libraries
if [ ! -e usr/include/numpy.hpp ]
then
    git clone https://gist.github.com/rezoo/5656056
    mkdir -p usr/include
    cp 5656056/numpy.hpp usr/include
fi


# setup tensorflow lite
git clone https://github.com/tensorflow/tensorflow.git tensorflow_src

pushd tensorflow_src
tfl_make=./tensorflow/lite/tools/make

if [ ! -e ${tfl_make}/downloads ]
then
    ${tfl_make}/download_dependencies.sh
fi

cp -a ../make/* ${tfl_make}

${tfl_make}/build_nerves_lib.sh

popd

