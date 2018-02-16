#ifndef CLI_PARSER_HPP_INCLUDED
#define CLI_PARSER_HPP_INCLUDED

#include "fractal_params.hpp"
#include "nvenc/nvenc_config.hpp"

namespace fractal
{
    void parse_command_line(int argc, char** argv, fractal_params_t& params, nvenc_encoder_config_t& cfg,const nvenc_encoder_limits_t& limits);
    void print_help();
}

#endif