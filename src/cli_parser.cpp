#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "cli_parser.hpp"

namespace fractal
{
    
    void exit_error(const char* err)
    {
        std::cerr<<err<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    class cli_parser_t
    {
        std::vector<std::string> arguments;
        std::string empty;
        
    public:
        
        cli_parser_t(int argc, char** argv)
        {
            arguments.reserve(argc);
            
            for(auto i=1;i<argc;++i)
            {
                arguments.emplace_back(argv[i]);
            }
        }
        
        bool option_triggered(const std::string& option)
        {
            return std::find(arguments.begin(),arguments.end(),option) != arguments.end();
        }
        
        const std::string& get_parameter(const std::string& option)
        {
            auto pos = std::find(arguments.begin(),arguments.end(),option);
            
            if (pos != arguments.end() && ++pos != arguments.end())
                return *pos;
                
            return empty;
        }
    };
    
    void parse_command_line(int argc, char** argv, fractal_params_t& params, nvenc_encoder_config_t& cfg, const nvenc_encoder_limits_t& limits)
    {
        cfg.height =  1080;
        cfg.width = 1920;
        cfg.fps = 30;
        cfg.number_of_b_frames = 4;
        params.height = cfg.height;
        params.width = cfg.width;
        params.fps = cfg.fps;
        params.number_of_b_frames = cfg.number_of_b_frames;

        cfg.gop_length = 64;
        cfg.vbr = true;
        cfg.avg_bitrate = 52428800u;
        cfg.max_bitrate = 209715200u;
        cfg.initial_qp = 25;
    
        params.x_0 = -2.978653333333335;
        params.y_0 = -1.424573000000001;
        params.x_1 = 4.148813333333335;
        params.y_1 = 2.5846270000000007;
        
        //params.x_0 = -7.28;
        //params.y_0 = -4;
        //params.x_1 = 8.72;
        //params.y_1 = 5;
        
        params.zoom = 4096;
        params.cutoff_value = 1.9;
        params.max_iterations_0 = 90;
        params.max_iterations_1 = 590;
        params.precision = precision_t::single_precision;
        params.duration = 10;
        
        params.threads_per_block_x = 16;
        params.threads_per_block_y = 16;
        
        params.output_path = "output.h264";
        
        params.silent = false;
        params.print_help = false;
        params.print_encoder_cap = false;
        
        cli_parser_t parser(argc,argv);

        
        if(parser.option_triggered("-cbr"))
            cfg.vbr = false;
        
        if(parser.option_triggered("-silent"))
            params.silent = true;
            
        if(parser.option_triggered("-single-precision"))
            params.precision = precision_t::single_precision;
        
        if(parser.option_triggered("-double-precision"))
            params.precision = precision_t::double_precision;
        
        if(parser.option_triggered("-help"))
            params.print_help = true;
        
        if(parser.option_triggered("-capabilities"))
            params.print_encoder_cap = true;
        
        std::string arg = parser.get_parameter("-size");
        
        if(arg.empty())
            arg = parser.get_parameter("-s");
        
        if(!arg.empty())
        {
            uint32_t height = 0;
            uint32_t width = 0;
            
            if (arg == "720p")
            {
                height = 720;
                width = 1024;
            }
            
            else if (arg == "1080p")
            {
                height = 1080;
                width = 1920;
            }
            
            else if (arg == "4K")
            {
                height = 2160;
                width = 3840;
            }
    
            else
            {
                bool bad_format = false;
            
                auto separator_pos = arg.find_first_of('x');
            
                if(separator_pos == std::string::npos)
                    bad_format = true;
                
                width = atoi(arg.substr(0,separator_pos).c_str());
                
                if(!bad_format)
                    height = atoi(arg.substr(separator_pos+1).c_str());
                
                if(height == 0 || width == 0 || width % 2 == 1 || height %2 == 1 || height > limits.max_height || width > limits.max_width)
                    bad_format = true;
                    
                if(bad_format)
                    exit_error("The video size provided is not valid!");            
            }
            
            params.height = height;
            params.width = width;
            cfg.height = params.height;
            cfg.width = params.width;
        }
        
        arg = parser.get_parameter("-rect");
        
        if(arg.empty())
            arg = parser.get_parameter("-r");
        
        if(!arg.empty())
        {
            bool bad_format = false;
            auto separator_pos = arg.find_first_of(':');
            char * end;
            
            params.x_0 = strtod(arg.c_str(),&end);
            
            if(separator_pos == std::string::npos)
                bad_format = true;
                
            if(!bad_format)
                arg = arg.substr(separator_pos+1);
            
            separator_pos = arg.find_first_of(':');
            params.x_1 = strtod(arg.c_str(),&end);
            
            
            if(separator_pos == std::string::npos)
                bad_format = true;
                
            if(!bad_format)
                arg = arg.substr(separator_pos+1);
                
            separator_pos = arg.find_first_of(':');
            params.y_0 = strtod(arg.c_str(),&end);
            
            
            if(separator_pos == std::string::npos)
                bad_format = true;
                
            if(!bad_format)
                arg = arg.substr(separator_pos+1);
                
            separator_pos = arg.find_first_of(':');
            params.y_1 = strtod(arg.c_str(),&end);
            
            if(params.x_0 - params.x_1 == 0.0 || params.y_0 - params.y_1 == 0.0 )
                bad_format = true;
            
            if(bad_format)
                exit_error("The coordinates of the complex rectangle are not valid!");
            
        }
        
         arg = parser.get_parameter("-fps");
         
         if(!arg.empty())
         {
             params.fps = atoi(arg.c_str());
             cfg.fps = params.fps;

             if(params.fps != 30 && params.fps != 60)
                 exit_error("The video fps value provided is not valid!");
         }
         
         arg = parser.get_parameter("-zoom");
         
         if(!arg.empty())
         {
             params.zoom = strtod(arg.c_str(),nullptr);
             
             if(params.zoom == 0.0)
                 exit_error("The value provided for zoom is not valid!");
         }
         
        arg = parser.get_parameter("-duration");
         
         if(!arg.empty())
         {
            params.duration = atoi(arg.c_str());
             if(params.duration < 1)
                 exit_error("The value provided for the video duration is not valid!");
         }
        
         arg = parser.get_parameter("-cutoff-value");
         
         if(!arg.empty())
         {
            params.cutoff_value = std::strtod(arg.c_str(), nullptr);
            
             if(params.cutoff_value < 1.3 )
                 exit_error("The cutoff value provided is not valid!");
         }
         
        arg = parser.get_parameter("-max-iterations-begin");
         
         if(!arg.empty())
         {
            params.max_iterations_0 = std::strtod(arg.c_str(), nullptr);
            
             if(params.max_iterations_0 < 30 )
                 exit_error("The value provided for the starting maximum iterations is too low!");
         }
         
        arg = parser.get_parameter("-max-iterations-end");
         
         if(!arg.empty())
         {
            params.max_iterations_1 = std::strtod(arg.c_str(), nullptr);
            
             if(params.max_iterations_1 < 30 )
                 exit_error("The value provided for the ending maximum iterations is too low!");
         }
         
        arg = parser.get_parameter("-avg-bitrate");
         
         if(!arg.empty())
         {
            cfg.avg_bitrate = atoi(arg.c_str());
            
             if(cfg.avg_bitrate == 0 )
                 exit_error("The value provided for the average bitrate is not valid!");
         }
         
        arg = parser.get_parameter("-max-bitrate");
         
         if(!arg.empty())
         {
            cfg.max_bitrate = atoi(arg.c_str());
            
             if(cfg.max_bitrate == 0 )
                 exit_error("The value provided for the maximum bitrate is not valid!");
         }
         
        arg = parser.get_parameter("-b-frames");
         
         if(!arg.empty())
         {
            cfg.number_of_b_frames = atoi(arg.c_str());
            params.number_of_b_frames = cfg.number_of_b_frames;
            
             if(cfg.number_of_b_frames > limits.max_b_frames )
                 exit_error("The number of B frames requested exceed the limits of the hardware encoder!");
         }
         
         
        arg = parser.get_parameter("-tpb");
         
         if(!arg.empty())
         {
            bool bad_format = false;
            
            auto separator_pos = arg.find_first_of('x');
            
            if(separator_pos == std::string::npos)
                bad_format = true;
                
               params.threads_per_block_x = atoi(arg.substr(0,separator_pos).c_str());
                
            if(!bad_format)
               params.threads_per_block_y = atoi(arg.substr(separator_pos+1).c_str());
               
            auto tpb = params.threads_per_block_x*params.threads_per_block_y;
                
            if(tpb != 1 && tpb % 32 != 0)
                bad_format = true;
                    
            if(bad_format)
                exit_error("Threads per block must be 1 or a multiple of 32!");   
         }
         
         

    }
    
    constexpr const char* help_string = 
    "Main Options: \n"
    "-help              Print usage details\n"
    "-silent            Don't show aditional timing information.\n"
    "-capabilities      Print HW h.264 encoder capabilities.\n"
    "\n"
    "Video Options:\n" 
    "-s (-size) NxM     Sets the video output size to NxM pixels.\n"
    "-fps N             Sets the frames per second to N\n"
    "-duration N        Sets the video duration to N\n"
    "-cbr               Force constant bitrate (vbr is the default option).\n"
    "-avg-bitrate N     Sets the average bitrate to N.\n"
    "-max-bitrate N     Sets the maximum bitrate to N (only useful in vbr mode).\n"
    "-b-frames N        Uses N B-frames for encoding.\n"
    "\n"
    "Rendering Options:\n"
    "-r (-rect) x_0:x_1:y_0:y_1     Starts with the [x_0,x_1]x[y_0,y_1] rectangle from the complex plane.\n"
    "-zoom N                        The last frame of the video is zoomed N times, gradually increasing zoom satring from the rectangle specified by -r.\n"
    "-max-iterations-begin N        Maximum iterations to check for divergence in the begining (the max-iterations are lerp of the begining and end max iterations"
    " using the current_frame/last_frame ratio).\n"
    "-max-terations-end N           Maximum iterations to check for divergence in the end (the max-iterations are lerp of the begining and end max iterations"
    " using the current_frame/last_frame ratio).\n"
    "-cutoff-value N                Value to wait the absolute value of a point to reach before concluding that the iterated sequence of points diverges.\n"
    "-tpb NxM                       Set's CUDA block dimensions to NxM."
    "\n";
    
    void print_help()
    {
        std::cout<<help_string<<std::endl;
    }
}