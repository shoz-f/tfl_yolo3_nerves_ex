/***  File Header  ************************************************************/
/**
* tfl_interp.cc
*
* Elixir/Erlang Port ext. of tensor flow lite
* @author	   Shozo Fukuda
* @date	create Sat Sep 26 06:26:30 JST 2020
* System	   MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/

#include <iostream>
#include <string>
#include <memory>
#include <iterator>
#include <regex>
using namespace std;

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <getopt.h>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
using namespace tflite;

#include "tfl_interp.h"
#include "tfl_helper.h"

/***  Global **************************************************************}}}*/
/**
* system infomation
**/
/**************************************************************************{{{*/
SysInfo gSys = {
    .mPortMode = false,
    .mNormalize = false,
    .mTiny      = false,
    .mDiag     = 0
};

/***  Type ****************************************************************}}}*/
/**
* convert "unsigned short" <-> "char[2]"
**/
/**************************************************************************{{{*/
union Magic {
    unsigned long L;
    char C[4];
};

/***  Module Header  ******************************************************}}}*/
/**
* receive command packet from Elixir/Erlang
* @par DESCRIPTION
*   receive command packet and store it to "cmd_line"
*
* @retval res >  0  success
* @retval res == 0  termination
* @retval res <  0  error
**/
/**************************************************************************{{{*/
ssize_t
rcv_packet_port(string& cmd_line)
{
    try {
        // receive packet size
        Magic len;
        /*+KNOWLEDGE:shoz:20/11/24:can't work "cin.get(len.C[3]).get(len.C[2]).." in WSL */
        len.C[3] = cin.get();
        len.C[2] = cin.get();
        len.C[1] = cin.get();
        len.C[0] = cin.get();

        // receive packet payload
        unique_ptr<char[]> buff(new char[len.L]);
        cin.read(buff.get(), len.L);

        // return received command line
        cmd_line.assign(buff.get(), len.L);
        return len.L;
    }
    catch(ios_base::failure) {
        return (cout.eof()) ? 0 : -1;
    }
    catch (std::bad_alloc& e) {
        cerr << e.what() << "@rcv_packet_port" << endl;
        return -1;
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* send result packet to Elixir/Erlang
* @par DESCRIPTION
*   construct message packet and send it to stdout
*
* @return count of sent byte or error code
**/
/**************************************************************************{{{*/
ssize_t
snd_packet_port(string result)
{
    try {
        Magic len = { static_cast<unsigned long>(result.size()) };
        (cout.put(len.C[3]).put(len.C[2]).put(len.C[1]).put(len.C[0]) << result).flush();
        return len.L;
    }
    catch(ios_base::failure) {
        return (cout.eof()) ? 0 : -1;
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* receive command packet from terminal
* @par DESCRIPTION
*   receive command packet and store it to "cmd_line"
*
* @retval res >  0  success
* @retval res == 0  termination
* @retval res <  0  error
**/
/**************************************************************************{{{*/
ssize_t
rcv_packet_terminal(string& cmd_line)
{
    try {
        cout.put('>').flush();
        getline(std::cin, cmd_line);
        return cmd_line.size();
    }
    catch(ios_base::failure) {
        return (cout.eof()) ? 0 : -1;
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* send result packet to terminal
* @par DESCRIPTION
*   construct message packet and send it to stdout
*
* @return count of sent byte or error code
**/
/**************************************************************************{{{*/
ssize_t
snd_packet_terminal(string result)
{
    try {
        std::cout << result << std::endl;
        return result.size();
    }
    catch(ios_base::failure) {
        return (cout.eof()) ? 0 : -1;
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* parse command line string
* @par DESCRIPTION
*   extract command & arguments strings from "cmd_line"
*
* @retval command string & vector of arguments
**/
/**************************************************************************{{{*/
string
parse_cmd_line(const string& cmd_line, vector<string>& args)
{
    regex reg(R"(\s+)");
    sregex_token_iterator iter(cmd_line.begin(), cmd_line.end(), reg, -1);
    sregex_token_iterator end;

    string command = *iter++;
    args.assign(iter, end);

    return command;
}

/***  Module Header  ******************************************************}}}*/
/**
* tensor flow lite interpreter
* @par DESCRIPTION
*   <<解説記入>>
**/
/**************************************************************************{{{*/
void
interp(string& tfl_model)
{
    // initialize tensor flow lite
    unique_ptr<tflite::FlatBufferModel> model =
        tflite::FlatBufferModel::BuildFromFile(tfl_model.c_str());

    tflite::ops::builtin::BuiltinOpResolver resolver;
    InterpreterBuilder builder(*model, resolver);
    unique_ptr<Interpreter> interpreter;
    builder(&interpreter);

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        cerr << "error: AllocateTensors()\n";
        exit(1);
    }

    // REPL
    for (;;) {
        // receive command packet
        string cmd_line;
        ssize_t n = gSys.mRcv(cmd_line);
        if (n <= 0) {
            break;
        }

        // parse command line
        vector<string> args;
        const string command = parse_cmd_line(cmd_line, args);

        // command branch
        json result;
        result.clear();
        if (command == "predict") {
            extern void predict(unique_ptr<Interpreter>& interpreter, const vector<string>& args, json& result);
            predict(interpreter, args, result);
        }
        else if (command == "info") {
            result["exe"]   = gSys.mExe;
            result["model"] = gSys.mTflModel;
            result["mode"]  = gSys.mPortMode ? "Ports" : "Terminal";
        }
        else {
            result["unknown"] = command;
        }

        // send the result in JSON string
        n = gSys.mSnd(result.dump());
        if (n <= 0) {
            break;
        }
    }
}

/***  Module Header  ******************************************************}}}*/
/**
* prit usage
* @par DESCRIPTION
*   print usage to terminal
**/
/**************************************************************************{{{*/
void usage()
{
    cout
      << "tfl_interp [opts] <model.tflite>\n"
      << "\toption:\n"
      << "\t  -p       : Elixir/Erlang Ports interface\n"
      << "\t  -n       : Normalize BBox predictions by 1.0x1.0\n"
      << "\t  -d <num> : diagnosis mode\n"
      << "\t             1 = save the formed image\n"
      << "\t             2 = save model's input/output tensors\n"
      << "\t             4 = save result of the prediction\n";
}

/***  Module Header  ******************************************************}}}*/
/**
* tensor flow lite for Elixir/Erlang Port ext.
* @par DESCRIPTION
*   Elixir/Erlang Port extension (experimental)
*
* @return exit status
**/
/**************************************************************************{{{*/
int
main(int argc, char* argv[])
{
    int opt, longindex;
    struct option longopts[] = {
        { "port",   no_argument,       NULL, 'p' },
        { "normal", no_argument,       NULL, 'n' },
        { "tiny",   no_argument,       NULL, 't' },
        { "debug",  required_argument, NULL, 'd' },
        { 0,        0,                 0,     0  },
    };

    for (;;) {
        opt = getopt_long(argc, argv, "d:npt", longopts, NULL);
        if (opt == -1) {
            break;
        }
        else switch (opt) {
        case 'p':
            gSys.mPortMode = true;
            break;
        case 'n':
            gSys.mNormalize = true;
            break;
        case 't':
            gSys.mTiny = true;
            break;
        case 'd':
            gSys.mDiag = atoi(optarg);
            break;
        case '?':
        case ':':
            cerr << "error: unknown options\n\n";
            usage();
            return 1;
        }
    }
    if ((argc - optind) < 1) {
        // argument error
        cerr << "error: expect <model.tflite>\n\n";
        usage();
        return 1;
    }

    // save exe infomations
    gSys.mExe.assign(argv[0]);
    gSys.mTflModel.assign(argv[optind]);

    // initialize i/o
    cin.exceptions(ios_base::badbit|ios_base::failbit|ios_base::eofbit);
    cout.exceptions(ios_base::badbit|ios_base::failbit|ios_base::eofbit);
    
    if (gSys.mPortMode) {
#ifdef _WIN32
        setmode(fileno(stdin),  O_BINARY);
        setmode(fileno(stdout), O_BINARY);
#endif
        gSys.mRcv = rcv_packet_port;
        gSys.mSnd = snd_packet_port;
    }
    else {
        gSys.mRcv = rcv_packet_terminal;
        gSys.mSnd = snd_packet_terminal;
    }

    // run interpreter
    interp(gSys.mTflModel);

    return 0;
}

/*** tfl_interp.cc ********************************************************}}}*/
