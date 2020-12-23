/***  File Header  ************************************************************/
/**
* tfl_helper.h
*
* helper functions for Tensorflow lite
* @author	   Shozo Fukuda
* @date	create Thu Nov 12 17:55:58 JST 2020
* System	   MINGW64/Windows 10<br>
*
*******************************************************************************/
#ifndef _TFL_HELPER_H
#define _TFL_HELPER_H

#include <string>
#include <numpy.hpp>
#include "tensorflow/lite/interpreter.h"

/**************************************************************************}}}**
* inline
***************************************************************************{{{*/
// get dimension size of the tensor
inline int size_of_dimension(const TfLiteTensor* t, int dim) {
    return (dim < t->dims->size) ? t->dims->data[dim] : -1;
}

// get typed tensor ptr
template <class T>
T* get_typed_tensor(TfLiteTensor* t) {
    return (t->type == tflite::typeToTfLiteType<T>()) ?
                reinterpret_cast<T*>(t->data.raw) : nullptr;
}

template <class T>
const T* get_typed_tensor(const TfLiteTensor* t) {
    return (t->type == tflite::typeToTfLiteType<T>()) ?
                reinterpret_cast<const T*>(t->data.raw) : nullptr;
}

// save the tensor in npy format
template <class T>
void save_tensor(const TfLiteTensor* t, std::string path) {
    aoba::SaveArrayAsNumpy<T>(
        path,
        false,
        t->dims->size,
        t->dims->data,
        get_typed_tensor<T>(t)
    );
}

// judge all elements of the array above threshold or not
inline bool any_one_above(int count, const float* values, float threshold) {
    for (int i = 0; i < count; i++) {
        if (values[i] >= threshold) { return true; }
    }
    return false;
}

// rounding up scaled x
inline int round(float x, float scale=1.0) {
    return int(x*scale + 0.5);
}

// get basename from path
inline std::string basename(const std::string& path, bool with_ext=true) {
    std::string fname = path.substr(path.rfind('/')+1);

    if (!with_ext && fname != "." && fname != "..") {
        return fname.substr(0, fname.rfind('.'));
    }
    else {
        return fname;
    }
}

#endif /* _TFL_HELPER_H */
