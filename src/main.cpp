#include <iostream>
#include <iomanip>
#include "video_renderer.hpp"
#include "nvenc/nvenc_encoder.hpp"
#include "cuda/cuda_context.hpp"
#include "cuda/cuda_module_loader.hpp"
#include "utility/timer.hpp"
#include "cli_parser.hpp"


fractal::cuda_context_t cuda;
fractal::nvenc_encoder_t nvenc((void*)cuda.context);
fractal::cuda_module_loader_t cuda_module_loader;

int main(int argc, char** argv)
{
    // TODO : Parse command line arguments and call render_video() with the parameters gathered
    
    fractal::nvenc_encoder_config_t cfg;
    fractal::fractal_params_t fp;
    
    fractal::nvenc_encoder_limits_t limits = nvenc.get_limits();
    
    parse_command_line(argc,argv,fp,cfg,limits);
    
    if(fp.print_help)
    {
        fractal::print_help();
        return 0;
    }
    
    nvenc.configure(cfg);
    
    if(fp.print_encoder_cap)
    {
        std::cout<<nvenc.get_capabilities()<<std::endl;
        return 0;
    }
    
    utility::timer_t timer;
    auto timings = fractal::render_video(fp);
    timer.stop();
    
    std::cout<<std::setprecision(2)<<std::fixed;
    
    if (!fp.silent)
    {
		std::cout<<"Minimum rendering time per frame : "<<timings.min_render_time_per_frame/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Average rendering time per frame : "<<timings.avg_render_time_per_frame/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Maximum rendering time per frame : "<<timings.max_render_time_per_frame/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Total rendering time : "<<timings.total_render_time/1E9<<" seconds."<<std::endl<<std::endl;
    
		std::cout<<"Minimum encoding time per input : "<<timings.min_encode_time_per_input/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Average encoding time per input : "<<timings.avg_encode_time_per_input/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Maximum encoding time per input : "<<timings.max_encode_time_per_input/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Total encoding time : "<<timings.total_encode_time/1E9<<" seconds."<<std::endl<<std::endl;
    
		std::cout<<"Minimum serialization time per output : "<<timings.min_serialization_time_per_output/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Average serialization time per output : "<<timings.avg_serialization_time_per_output/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Maximum serialization time per output : "<<timings.max_serialization_time_per_output/1E6<<" milliseconds."<<std::endl;
		std::cout<<"Total serialization time : "<<timings.total_serialization_time/1E9<<" seconds."<<std::endl<<std::endl;
	}
	
    std::cout<<"Total running time : "<<timer.get_duration()/1E9<<" seconds."<<std::endl;
    
    
    return 0;
}
