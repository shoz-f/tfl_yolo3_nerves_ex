/***  File Header  ************************************************************/
/**
* tfl_interp.h
*
* system setting - used throughout the system
* @author	   Shozo Fukuda
* @date	create Thu Nov 12 17:55:58 JST 2020
* System	   MINGW64/Windows 10<br>
*
*******************************************************************************/
#ifndef _TFL_INTERP_H
#define _TFL_INTERP_H

#include <string>

/**************************************************************************}}}**
* system information
***************************************************************************{{{*/
struct SysInfo {
    std::string    mExe;       // path of this executable
    std::string    mTflModel;  // path of Tflite Model
    bool           mPortMode;  // Ports [true] or Terminal [false] flag
    bool           mNormalize; // Normalize BBox predictions [true] or not
    bool           mTiny;      // Yolo V3 tiny model
    unsigned long mDiag;      // diagnosis mode

    // i/o method
    ssize_t (*mRcv)(string& cmd_line);
    ssize_t (*mSnd)(string  result);
};
extern SysInfo gSys;

/**************************************************************************}}}**
* control debug output
***************************************************************************{{{*/
#if 1
#  define DIAG(x)    if (gSys.mDiag & (1 << (x)))
#else
#  define DIAG(x)    if (0)
#endif
#define DIAG_FORMED_IMG      DIAG(0)
#define DIAG_IN_OUT_TENSOR   DIAG(1)
#define DIAG_RESULT          DIAG(2)


#endif /* _TFL_INTERP_H */
