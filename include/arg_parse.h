#ifndef SSM_ARG_PARSE_H
#define SSM_ARG_PARSE_H
#include <string>
#include "user_input.h"
#include <stdexcept>
class ArgParser
{
private:
    bool parse_args(int argc, char *argv[]);

public:
    ArgParser(int argc, char *argv[]);
    InputMode input_mode;
};

#endif