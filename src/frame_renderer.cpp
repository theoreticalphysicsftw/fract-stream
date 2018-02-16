#include <complex>
#include <cmath>
#include "frame_renderer.hpp"
#include "cuda/cuda_module_loader.hpp"

extern fractal::cuda_module_loader_t cuda_module_loader;

namespace fractal
{
    void render_frame(const fractal_params_t& fractal_params, frame_buffer_t& buffer, size_t frame_idx)
    {
        // We have to calculate the new rectangle of the complex frame for the zoomed frame.
        
        typedef std::complex<double> complex_t;
        
        size_t max_frame_idx = fractal_params.fps*fractal_params.duration-1;
        double frame_idx_ratio = double(frame_idx)/double(max_frame_idx);
        // The scale that has to be applied to the rectangle it's reciprocal of the zoom
        double scale = (1.0 - frame_idx_ratio)*((1.0 - frame_idx_ratio + frame_idx_ratio/fractal_params.zoom) *(1 - frame_idx_ratio) + 
                        frame_idx_ratio*(1.0/(1.0 - frame_idx_ratio + frame_idx_ratio*fractal_params.zoom))) + 
                        frame_idx_ratio * std::pow(fractal_params.zoom, -frame_idx_ratio);
                        
        unsigned int max_iterations = (unsigned int)(fractal_params.max_iterations_0*(1.0-frame_idx_ratio) + fractal_params.max_iterations_1*frame_idx_ratio);
        
        // Top right corner
        complex_t max_pt(fractal_params.x_1,fractal_params.y_1);
        // Bottom left corner
        complex_t min_pt(fractal_params.x_0,fractal_params.y_0);
        complex_t center = ( min_pt + max_pt )/ 2.0;
        
        // Now we can scale and get the new coordinates of the rectangle;
        max_pt = scale*(max_pt - center) + center;
        min_pt = scale*(min_pt - center) + center;
        
        unsigned int height = fractal_params.height;
        unsigned int width = fractal_params.width;
        unsigned int stride = buffer.nv12_frame_stride;
        
        unsigned int threads_per_block_x = fractal_params.threads_per_block_x;
        unsigned int threads_per_block_y = fractal_params.threads_per_block_y;
        
        unsigned int number_of_blocks_x = width/threads_per_block_x + 1;
        unsigned int number_of_blocks_y = height/threads_per_block_y + 1;
        
        if(fractal_params.precision == precision_t::double_precision)
        {
            double x_0 = min_pt.real();
            double x_step = (max_pt.real() - min_pt.real())/width;
            double y_1 = max_pt.imag();
            double y_step = (max_pt.imag() - min_pt.imag())/height;
            double cutoff_value_sq = fractal_params.cutoff_value*fractal_params.cutoff_value;
            
            void * kernel_params[] =
            {
                &buffer.nv12_frame,
                &height,
                &width,
                &stride,
                &x_0,
                &x_step,
                &y_1,
                &y_step,
                &cutoff_value_sq,
                &max_iterations
            };
            
            cuda_module_loader.execute_double(kernel_params,number_of_blocks_x,number_of_blocks_y,threads_per_block_x,threads_per_block_y);
        }
        else
        {
            float x_0 = min_pt.real();
            float x_step = (max_pt.real() - min_pt.real())/width;
            float y_1 = max_pt.imag();
            float y_step = (max_pt.imag() - min_pt.imag())/height;
            float cutoff_value_sq = fractal_params.cutoff_value*fractal_params.cutoff_value;
            
            void * kernel_params[] =
            {
                &buffer.nv12_frame,
                &height,
                &width,
                &stride,
                &x_0,
                &x_step,
                &y_1,
                &y_step,
                &cutoff_value_sq,
                &max_iterations
            };
            
            cuda_module_loader.execute_float(kernel_params,number_of_blocks_x,number_of_blocks_y,threads_per_block_x,threads_per_block_y);
            
        }
        
    }
    
}