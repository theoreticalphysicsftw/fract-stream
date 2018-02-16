#include <functional>
#include <limits>
#include "video_renderer.hpp"
#include "fractal_params.hpp"
#include "nvenc/frame_buffer.hpp"
#include "cuda/cuda_context.hpp"
#include "frame_renderer.hpp"



extern fractal::cuda_context_t cuda;

namespace fractal
{
    // TODO: Implement the main logic and complement all the other components on the fly
    
    void render(const fractal_params_t& fractal_params,frame_queue_t& frame_queue,timings_t& timings)
    {
        // Use the cuda context for this thread
        ::cuda.use_context_for_this_thread();
        
        utility::timer_t timer;
        
        for(auto i=0;i<fractal_params.duration*fractal_params.fps;++i)
        {
            frame_buffer_t& buffer = frame_queue.get_for_rendering();
            
            timer.start();
            render_frame(fractal_params,buffer,i);
            timer.stop();
            
            buffer.map();
            
            frame_queue.end_rendering_frame();
            
            auto time = timer.get_duration();
            
            if(time < timings.min_render_time_per_frame)
                timings.min_render_time_per_frame = time;
                
            if(time > timings.max_render_time_per_frame)
                timings.max_render_time_per_frame = time;
                
            timings.total_render_time += time;
        }
        
        timings.avg_render_time_per_frame = timings.total_render_time/(fractal_params.duration*fractal_params.fps);
    }
    
    void encode(const fractal_params_t& fractal_params,frame_queue_t& frame_queue,timings_t& timings)
    {
        auto inputs_encoded = 0;
        utility::timer_t timer;
        
        for(auto i=0;i<fractal_params.duration*fractal_params.fps;++i)
        {
            frame_buffer_t& buffer = frame_queue.get_for_encoding();
            
            timer.start();
            auto dont_need_more_input = buffer.encode();
            timer.stop();
            
            frame_queue.end_encoding_frame();
            
            auto time = timer.get_duration();
            
            if(dont_need_more_input)
            {
                frame_queue.signal_serialization();
                inputs_encoded++;
                
                
                if(time < timings.min_encode_time_per_input)
                    timings.min_encode_time_per_input = time;
                
                if(time > timings.max_encode_time_per_input)
                    timings.max_encode_time_per_input = time;
                    
                
            }
            
            timings.total_encode_time += time;
            
        }
        // End Of Stream
        frame_buffer_t& buffer = frame_queue.get_for_rendering();
        buffer.encode_eos();
        frame_queue.signal_serialization();
        
        timings.avg_encode_time_per_input = timings.total_encode_time / inputs_encoded;
    }
    
    void serialize(const fractal_params_t& fractal_params,frame_queue_t& frame_queue, timings_t& timings)
    {
        FILE* file = fopen(fractal_params.output_path.c_str(),"wb");
        utility::timer_t timer;
        
        for(auto i=0;i<fractal_params.duration*fractal_params.fps;++i)
        {
            frame_buffer_t& buffer = frame_queue.get_for_serialization();
            void* ptr;
            uint32_t size;
            
            timer.start();
            buffer.lock_output(ptr,size);
            fwrite(ptr,1,size,file);
            buffer.unlock_output();
            timer.stop();
            
            buffer.unmap();
            frame_queue.end_serializing_frame();
            
            auto time = timer.get_duration();
            
            if(time < timings.min_serialization_time_per_output)
                timings.min_serialization_time_per_output = time;
                
            if(time > timings.max_serialization_time_per_output)
                timings.max_serialization_time_per_output = time;
            
            timings.total_serialization_time += time;
        }
        
        timings.avg_serialization_time_per_output = timings.total_serialization_time/(fractal_params.duration*fractal_params.fps);
    }
    
    timings_t render_video(const fractal_params_t& fractal_params)
    {
        
        frame_queue_t frame_queue(fractal_params);
        
        timings_t timings;
        memset(&timings,0,sizeof(timings));
        timings.min_render_time_per_frame = std::numeric_limits<timings_t::duration_t>::max();
        timings.min_encode_time_per_input = std::numeric_limits<timings_t::duration_t>::max();
        timings.min_serialization_time_per_output = std::numeric_limits<timings_t::duration_t>::max();
        timings.max_render_time_per_frame = std::numeric_limits<timings_t::duration_t>::min();
        timings.max_encode_time_per_input = std::numeric_limits<timings_t::duration_t>::min();
        timings.max_serialization_time_per_output = std::numeric_limits<timings_t::duration_t>::min();
        
        // Use std::ref to return std::reference_wraper because std::thread passes by value
        std::thread rendering_thread(render, fractal_params,std::ref(frame_queue),std::ref(timings));
        std::thread encoding_thread(encode, fractal_params,std::ref(frame_queue),std::ref(timings));
        std::thread serialization_thread(serialize, fractal_params,std::ref(frame_queue),std::ref(timings));
        
        rendering_thread.join();
        encoding_thread.join();
        serialization_thread.join();
        
        return timings;
    }
    

    
    
}