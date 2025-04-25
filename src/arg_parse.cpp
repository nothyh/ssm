#include <string>
#include "arg_parse.h"
#include "user_input.h"

ArgParser::ArgParser(int argc, char *argv[])
{
    if (!parse_args(argc, argv))
    {
        throw std::runtime_error("Error parsing arguments");
    }
}

bool ArgParser::parse_args(int argc, char *argv[])
{
    if (argc < 2)
    {
        this->input_mode = InputMode::CLI;
        return true;
    }
    std::string option = argv[1];
    if (option == "-c")
    {
        this->input_mode = InputMode::CLI;
    }
    return true;
}