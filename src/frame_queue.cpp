#include "frame_queue.hpp"

namespace fractal
{
    frame_queue_t::
    frame_queue_t(const fractal_params_t& params) :
                                                for_rendering_index(0),
                                                for_encoding_count(0),
                                                for_encoding_index(0),
                                                for_serialization_count(0),
                                                for_serialization_index(0),
                                                ready_for_serialization_count(0)
    {
        auto queue_size=4+params.number_of_b_frames;
        queue.reserve(queue_size);
        
        for(auto i=0;i<queue_size;++i)
            queue.emplace_back(params.height,params.width);
    }
    
    
    frame_buffer_t& frame_queue_t::get_for_rendering()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        while(for_encoding_count+for_serialization_count>=queue.size())
                wait_for_render_buffer.wait(lock);
                
        lock.unlock();
        
        return queue[for_rendering_index];
    }
    
    
    void frame_queue_t::end_rendering_frame()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        for_rendering_index = (for_rendering_index + 1) % queue.size();
        for_encoding_count++;
        
        lock.unlock();
        
        wait_for_encode_buffer.notify_all();
    }
    
    
    frame_buffer_t& frame_queue_t::get_for_encoding()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        while(!for_encoding_count)
            wait_for_encode_buffer.wait(lock);
            
        lock.unlock();
        
        return queue[for_encoding_index];
    }
    
    
    void frame_queue_t::end_encoding_frame()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        for_encoding_count--;
        for_encoding_index = (for_encoding_index + 1) % queue.size();
        for_serialization_count++;
        
        lock.unlock();
    }
    
    // Call after end_encoding_frame and success when calling encode() on buffer
    void frame_queue_t::signal_serialization()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        auto sz=queue.size();
        
        ready_for_serialization_count = (for_encoding_index - for_serialization_index + sz)% sz;
        
        lock.unlock();
        
        wait_for_serialization_buffer.notify_all();
    }
    
    
    frame_buffer_t& frame_queue_t::get_for_serialization()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        while(!ready_for_serialization_count)
            wait_for_serialization_buffer.wait(lock);
            
        lock.unlock();
        
        return queue[for_serialization_index];
    }
    
    
    void frame_queue_t::end_serializing_frame()
    {
        std::unique_lock<std::mutex> lock(mutex);
        
        for_serialization_count--;
        ready_for_serialization_count--;
        for_serialization_index = (for_serialization_index + 1) % queue.size();
        
        lock.unlock();
        
        wait_for_render_buffer.notify_all();
    }
    
    
}