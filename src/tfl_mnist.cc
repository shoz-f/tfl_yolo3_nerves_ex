/***  File Header  ************************************************************/
/**
* tfl_mnist.cc
*
* tensor flow lite prediction
* @author	   Shozo Fukuda
* @date	create Sat Sep 26 06:26:30 JST 2020
* System	   MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/
#ifndef cimg_plugin

#define cimg_plugin     "tfl_mnist.cc"
#define cimg_display    0
#define cimg_use_jpeg
#include "CImg.h"
using namespace cimg_library;

#include <string>
using namespace std;

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "tensorflow/lite/interpreter.h"
using namespace tflite;

#include "tfl_helper.h"

/***  Module Header  ******************************************************}}}*/
/**
* parse command line string
* @par DESCRIPTION
*   extract command & arguments string from string
*
* @retval command string & vector of arguments
**/
/**************************************************************************{{{*/
void
predict(unique_ptr<Interpreter>& interpreter, const vector<string>& args, json& result)
{
/*PRECONDITION*/
    if (args.size() < 1) {
        result["error"] = "not enough argument";
        return;
    }
/**/

    // setup the input tensor.
    float* input = interpreter->typed_input_tensor<float>(0);
    int width  = size_of_dimension(interpreter->tensor(0), 1);
    int height = size_of_dimension(interpreter->tensor(0), 2);
    float scale[2];

    typedef CImg<unsigned char> CImgU8;
    try {
        CImgU8 img(args[0].c_str());

        scale[0] = float(img.width())  / width;
        scale[1] = float(img.height()) / height;

        // convert the image to required format.
        auto formed_img = img.getRGBtoGRAY(CImgU8::cNEGA).resize(width, height);

        // put the formed image into the input tensor.
        cimg_forXY(formed_img, x, y) {
        cimg_forC(formed_img, c) {
            *input++ = formed_img(x, y, c)/255.0;
        }}
    }
    catch (...) {
        result["error"] = "fail CImg";
        return;
    }
    
    // predict
    if (interpreter->Invoke() == kTfLiteOk) {
        // get result
        float* probs = interpreter->typed_output_tensor<float>(0);
        for (int i = 0; i < 10; i++) {
            result[to_string(i)] = probs[i];
        }
    }
    else {
        result["error"] = "fail predict";
    }
}

#else
/**************************************************************************}}}*/
/*** CImg Plugins:                                                          ***/
/**************************************************************************{{{*/
// option: image convert POSI/NEGA 
enum {
    cPOSI = 0,
    cNEGA
};

// get a GRAY converted image
CImg<T> getRGBtoGRAY(int optPN=cPOSI)
{
    if (_spectrum != 3) {
        throw CImgInstanceException(_cimg_instance
                                    "getRGBtoGRAY(): Instance is not a RGB image.",
                                    cimg_instance);
    }
    CImg<T> res(width(), height(), depth(), 1);
    T *p1 = data(0,0,0,0), *p2 = data(0,0,0,1), *p3 = data(0,0,0,2), *Y = res.data(0,0,0,0);
    const longT whd = (longT)width()*height()*depth();
    cimg_pragma_openmp(parallel for cimg_openmp_if_size(whd,256))
    for (longT i = 0; i < whd; i++) {
        const T
          R = p1[i],
          G = p2[i],
          B = p3[i];
        Y[i] = (T)(0.299f*R + 0.587f*G + 0.114f*B);

        if (optPN == cNEGA) {
            Y[i] = cimg::type<T>::max() - Y[i];
        }
    }
    return res;
}
#endif
/*** tfl_predict.cc *******************************************************}}}*/