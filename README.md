# Tensorflow lite YOLO v3 for Elixir

Tensorflow lite YOLO v3 application in Elixir

## Platform
- Windows MSYS2/MinGW64
- Ubuntu on WSL2

## Requirement
It needs following libraries to build this application.

#### Libraries available as a binary package:
- libjpeg
- libdl
- nlohmann/json: JSON for Modern C++

You can install these libraries by the packge manager, pacman, apt and so on.

MSYS/MinGW64:

```bash
$ pacman -S mingw-w64-x86_64-libjpeg-turbo
$ pacman -S mingw-w64-x86_64-dlfcn
$ pacman -S mingw-w64-x86_64-nlohmann-json
```

#### Libraries in source package:
- CImg:           http://cimg.eu/download.shtml
- tensorflow_src: https://github.com/tensorflow/tensorflow.git
- numpy.hpp:      https://gist.github.com/rezoo/5656056

You get these libraries in source and need to build them.

There is the installation script in "./extra" for your convenience.<br>
It downloads the source from internet, builds them and put them in suitable directories.<br>
You just run following command line depending on your OS.

MSYS2/MinGW64:

```bash
$ cd ./extra
$ source setup_mingw_extra.sh
```

Ubuntu on WSL2:

```bash
$ cd ./extra
$ source setup_wsl_extra.sh
```

## Where can i get YOLO v3 model for Tensorflow lite?
You can find the "yolov3-416.tflite" on the Release page in this repository.<br>
Donwload it and put it into "./priv" of the project.

Or, you can convert the Tensorflow YOLO v3 model to tflite also.

## About tfl_interp executable
"tfl_interp" is stand alone executable. It means that you can run "tfl_interp"
in your terminal without Elixir. It's comand line usage is here:

```
tfl_interp [opts] <model.tflite>
  <model.tflite>  -  Tensorflow lite model file
  [opts]
    -p       : switch to Elixir Ports mode from Terminal mode
    -d <num> : diagnosis output (ableable in both teminal/ports mode)
               num = 1 - save formed image (resized 416x416 in YOLO v3)
                     2 - save model's input/ouput tensors (npy format)
```

In teminal mode, without "-p" option, "tfl_interp" puts a prompt sign ">" on your terminal
and you input subcommands in it. 

```
SUBCOMMANDs:
 1) predict <image.jpg> - run prediction with <image.jpg>
```

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed
by adding `tfl_yolo3` to your list of dependencies in `mix.exs`:

```elixir
def deps do
  [
    {:tfl_yolo3, "~> 0.1.0"}
  ]
end
```
