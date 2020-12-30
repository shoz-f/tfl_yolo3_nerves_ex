/***  File Header  ************************************************************/
/**
* tfl_yolo3.cc
*
* tensor flow lite prediction YOLO v3
* @author	   Shozo Fukuda
* @date	create Sun Nov 01 22:01:42 JST 2020
* System	   MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/
#ifndef cimg_plugin

#define cimg_plugin     "tfl_yolo3.cc"
#define cimg_display    0
#define cimg_use_jpeg
#include "CImg.h"
using namespace cimg_library;

#include <string>
#include <list>
#include <vector>
#include <algorithm>
using namespace std;

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "tensorflow/lite/interpreter.h"
using namespace tflite;

#include "tfl_interp.h"
#include "tfl_helper.h"

#include "coco.names"

/***  Class Header  *******************************************************}}}*/
/**
* bounding box
* @par DESCRIPTION
*   it holds bbox and scores needed for NMS and provides IOU function.
**/
/**************************************************************************{{{*/
class Box {
//CONSTANT:
public:

//LIFECYCLE:
public:
    Box(const float* scores, const float box[4]) {
        mScore  = scores;
        mBBox[0] = box[0] - box[2]/2.0;
        mBBox[1] = box[1] - box[3]/2.0;
        mBBox[2] = box[0] + box[2]/2.0;
        mBBox[3] = box[1] + box[3]/2.0;
        mArea   = box[2]*box[3];
    }

//ACTION:
public:
    // calc Intersection over Union
    float iou(const Box& x) const {
        float x1 = min(mBBox[0], x.mBBox[0]);
        float y1 = min(mBBox[1], x.mBBox[1]);
        float x2 = max(mBBox[2], x.mBBox[2]);
        float y2 = max(mBBox[3], x.mBBox[3]);
        
        if (x1 < x2 && y1 < y2) {
            float v_intersection = (x2 - x1)*(y2 - y1);
            float v_union        = mArea + x.mArea - v_intersection;
            return v_intersection/v_union;
        }
        else {
            return 0.0;
        }
    }

    // put out the scaled BBox in JSON formatting
    json put_json(float scale_x=1.0, float scale_y=1.0) const {
        auto result = json::array();
        if (gSys.mNormalize) {
            result.push_back(mBBox[0]*scale_x);
            result.push_back(mBBox[1]*scale_y);
            result.push_back(mBBox[2]*scale_x);
            result.push_back(mBBox[3]*scale_y);
        }
        else {
        result.push_back(round(mBBox[0], scale_x));
        result.push_back(round(mBBox[1], scale_y));
        result.push_back(round(mBBox[2], scale_x));
        result.push_back(round(mBBox[3], scale_y));
        }
        return result;
    }

//ACCESSOR:
public:
    float score(int class_id) const {
        return mScore[class_id];
    }

//INQUIRY:
public:
    
//PRINT;
public:
    ostream& put(ostream& s) const
    {
        return s
            << "score: [" << mScore[0] << "," << mScore[1] << "," << mScore[2] << ", ...]\n"
            << "box  : [" << mBBox[0] << "," << mBBox[1] << "," << mBBox[2] << "," << mBBox[3] << "]\n"
            << "area : " << mArea << "\n";
    }
    
//ATTRIBUTE:
protected:
    const float* mScore;
    float  mBBox[4];
    float  mArea;
};

ostream& operator<<(ostream& s, const Box& box)
{
    return box.put(s);
}

// TYPE: list of Box
typedef list<const Box*> Boxes;

/***  Module Header  ******************************************************}}}*/
/**
* Non Maximum Suppression
* @par DESCRIPTION
*   run non-maximum on the sepcified class
*
* @retval list of predicted boxes
**/
/**************************************************************************{{{*/
Boxes
non_maximum_suppression(int class_id, const vector<Box>& db_candidates, float threshold=0.25, float iou_threshold=0.5)
{
    // pick up box from db_candidates and make a list with it in order of score.
    Boxes prior;
    for (const auto& box : db_candidates) {
        float my_score = box.score(class_id);
        if (my_score < threshold) { continue; }

        auto pos = find_if(prior.begin(), prior.end(), [class_id, my_score](const Box* x) {
            return x->score(class_id) <= my_score;
        });
        prior.insert(pos, &box);
    }
    
    // remove overlaped boxes by IOU
    Boxes result;
    if (!prior.empty()) {
        do {
            auto highest = prior.front();  prior.pop_front();

            result.push_back(highest);

            prior.remove_if([&highest, iou_threshold](const Box* x) {
                return highest->iou(*x) >= iou_threshold;
            });
        } while (!prior.empty());
    }
    return result;
}

/***  Module Header  ******************************************************}}}*/
/**
* post-processing for yolo3
* @par DESCRIPTION
*   filtering noize and run Non-Maximum-Suppression
*
* @retval command string & vector of arguments
**/
/**************************************************************************{{{*/
void
post_yolo3(json& result, int count, const float* boxes, const float* scores, float scale[2], float threshold=0.25, float iou_threshold=0.5)
{
    // leave only candidates above the threshold.
    vector<Box> db_candidates;
    for (int i = 0; i < count; i++, boxes += 4, scores += COCO_NAMES_MAX) {
        if (any_one_above(COCO_NAMES_MAX, scores, threshold)) {
            db_candidates.emplace_back(scores, boxes);
        }
    }
    
    // run nms over each classification class.
    bool nothing = true;
    for (int class_id = 0; class_id < COCO_NAMES_MAX; class_id++) {
        auto res = non_maximum_suppression(class_id, db_candidates, threshold, iou_threshold);
        if (res.empty()) { continue; }

        nothing = false;

        auto jboxes = json::array();
        for (const auto* box : res) {
            jboxes.push_back(box->put_json(scale[0], scale[1]));
        }
        result[gCocoNames[class_id]] = jboxes;
    }
    if (nothing) {
        result["error"] = "can't find any objects";
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* predict image by yolo3
* @par DESCRIPTION
*   object detection
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
    
    // get basename of image file
    string base = "/root/"+basename(args[0], false);

    // setup the input tensor.
    TfLiteTensor* itensor0 = interpreter->input_tensor(0);
    int width  = size_of_dimension(itensor0, 1);
    int height = size_of_dimension(itensor0, 2);
    float scale[2];

    typedef CImg<unsigned char> CImgU8;
    try {
        // load target image
        CImgU8 img(args[0].c_str());

        // save image sacle factor for post process
        if (gSys.mNormalize) {
            scale[0] = 1.0 / width;
            scale[1] = 1.0 / height;
        }
        else {
            scale[0] = float(img.width())  / width;
            scale[1] = float(img.height()) / height;
        }

        // convert the image to required format.
        auto formed_img = img.get_resize(width, height);
        
        DIAG_FORMED_IMG {
            formed_img.save((base+"_formed.jpg").c_str());
        }

        // put the formed image into the input tensor.
        float* input = get_typed_tensor<float>(itensor0);

        cimg_forXY(formed_img, x, y) {
        cimg_forC(formed_img, c) {
            // normalize the intensity of pixel and set it to the input tensor
            *input++ = formed_img(x, y, c)/255.0;
        }}
        
        DIAG_IN_OUT_TENSOR {
            save_tensor<float>(itensor0, base+"_input.npy");
        }
    }
    catch (...) {
        result["error"] = "fail CImg";
        return;
    }
    
    // predict
    if (interpreter->Invoke() == kTfLiteOk) {
        // get result from the output tensors
        const TfLiteTensor *otensor0, *otensor1;
        if (gSys.mTiny) {
            // tiny model results
            otensor0 = interpreter->output_tensor(1);   // BBOX
            otensor1 = interpreter->output_tensor(0);   // score each COCOs
        }
        else {
            // full model results
            otensor0 = interpreter->output_tensor(0);   // BBOX
            otensor1 = interpreter->output_tensor(1);   // score each COCOs
        }

        DIAG_IN_OUT_TENSOR {
            save_tensor<float>(otensor0, base+"_output0.npy");
            save_tensor<float>(otensor1, base+"_output1.npy");
        }

        // do post processing
        post_yolo3(
            result,
            size_of_dimension(otensor0, 1),
            get_typed_tensor<float>(otensor0),
            get_typed_tensor<float>(otensor1),
            scale,
            0.25,
            0.5
        );
    }
    else {
        result["error"] = "fail predict";
    }
    DIAG_RESULT {
        std::ofstream save_result(base+"_result.txt");
        save_result << result.dump();
    }
}

#else
/**************************************************************************}}}*/
/*** CImg Plugins:                                                          ***/
/**************************************************************************{{{*/
#endif
/*** tfl_yolo3.cc *******************************************************}}}*/