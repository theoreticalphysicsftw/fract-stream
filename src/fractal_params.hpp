#ifndef FRACTAL_PARAMS_HPP_INCLUDED
#define FRACTAL_PARAMS_HPP_INCLUDED

#include <cstdint>
#include <string>

// TODO : add the rest of the parameters lazily (when they are needed in other functions)

namespace fractal
{
    enum class precision_t : uint32_t 
    {
        single_precision,
        double_precision
    };
    
    struct fractal_params_t
    {
        std::string output_path;
        
        // Coordinates [x_0,x_1] , [y_0,y_1] in the complex plane
        double x_0;
        double x_1;
        double y_0;
        double y_1;
        
        double cutoff_value;
        
        double zoom;
        
        uint32_t threads_per_block_x;
        uint32_t threads_per_block_y;
        uint32_t max_iterations_0;
        uint32_t max_iterations_1;
        
        // Resolution of the video
        uint32_t height;
        uint32_t width;
        uint32_t duration;
        uint32_t fps;
        uint32_t number_of_b_frames;
        precision_t precision;
        
        bool silent;
        bool print_encoder_cap;
        bool print_help;
    };
}

#endif