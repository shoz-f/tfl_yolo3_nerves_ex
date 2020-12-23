proj_top=`pwd`/..

# setup depend packages
if [ ! -e ./usr/include/CImg.h ]
then
    wget -nc http://cimg.eu/files/CImg_latest.zip
    unzip CImg_latest.zip
    mkdir -p usr/include
    cp    CImg-*/CImg.h  usr/include
    cp -r CImg-*/plugins usr/include
fi

if [ ! -e ./usr/include/numpy.hpp ]
then
    git clone https://gist.github.com/rezoo/5656056
    cp ./5656056/numpy.hpp ./usr/include
fi

# setup tensorflow lite
git clone https://github.com/tensorflow/tensorflow.git tensorflow_src
patch -c tensorflow_src/tensorflow/lite/tools/make/Makefile tfl_make_mingw.patch

pushd tensorflow_src
tfl_make=./tensorflow/lite/tools/make

if [ ! -e ${tfl_make}/downloads ]
then
    ${tfl_make}/download_dependencies.sh
fi

${tfl_make}/build_lib.sh

popd
