#ifndef TIMER_HPP_INCLUDED
#define TIMER_HPP_INCLUDED

#include <chrono>

namespace utility
{
    class timer_t
    {
    public:
        
        typedef typename std::chrono::high_resolution_clock::time_point time_point_t;
        typedef typename std::chrono::nanoseconds::rep duration_t;
        
    private:
    
        time_point_t begin;
        time_point_t end;
        
    public:
    
        timer_t() : begin(std::chrono::high_resolution_clock::now()) {}
        inline void start() { begin = std::chrono::high_resolution_clock::now(); }
        inline void stop() { end = std::chrono::high_resolution_clock::now(); }
        inline duration_t get_duration() { return std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count(); }
    };
}

#endif