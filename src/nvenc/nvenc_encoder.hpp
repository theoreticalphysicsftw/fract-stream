#ifndef NVENC_ENCODER_HPP_INCLUDED
#define NVENC_ENCODER_HPP_INCLUDED

#include <cstring>
#include <string>
#include <nvEncodeAPI.h>
#include "../utility/shared_lib_loader.hpp"
#include "nvenc_config.hpp"
// TODO : Implement the rest of the functionality (actual encoding)

namespace fractal
{
	class nvenc_encoder_t
    {
    public:
    
        typedef NVENCSTATUS (NVENCAPI *nvenc_init_func_t)(NV_ENCODE_API_FUNCTION_LIST*);
        typedef NV_ENCODE_API_FUNCTION_LIST nvenc_api_t;
        typedef NV_ENC_INITIALIZE_PARAMS nvenc_init_params_t;
        typedef NV_ENC_CONFIG nvenc_config_t;
        typedef NV_ENC_OUTPUT_PTR output_ptr_t;
        typedef NV_ENC_INPUT_PTR input_ptr_t;
        typedef GUID guid_t;
        
    private:
        // Configuration parameters
        nvenc_init_params_t init_params;
        nvenc_config_t nvenc_config;
        
        nvenc_api_t* nvenc_api;
        utility::dll_handle_t nvenc_dll;
        
        // This is the device handle for nvenc
        void* encoder;
        guid_t encoder_guid;
        guid_t preset_guid;
        
        uint64_t frame_time_stamp;
        
        bool successful_init; // Not used at the moment
        
    public:
    
        nvenc_encoder_t(void* cuda_device);
        ~nvenc_encoder_t();
        
        bool configure(const nvenc_encoder_config_t& cfg);
        // uses height and width from the configuration
        bool register_cuda_devptr(void* devptr, void*& registred_resource,size_t stride);
        bool unregister_cuda_devptr(void* resource);
        bool create_bitstream_buffer(output_ptr_t& buffer);
        bool destroy_bitstream_buffer(output_ptr_t buffer);
        bool map_input_resource(void* registered_resource, input_ptr_t&);
        bool unmap_input_resource(input_ptr_t);
        bool encode_frame(input_ptr_t, output_ptr_t, size_t stride, bool eos);
        bool lock_bitstream_buffer(output_ptr_t out_buffer, void*& bitstream_cpu_ptr, uint32_t& bitstream_size);
        bool unlock_bitstream_buffer(output_ptr_t out_buffer);
        
        nvenc_encoder_limits_t get_limits();
        std::string get_capabilities();
        
    private:
    
        bool load_library();
        bool create_enc_session(void* cuda_device);
        bool select_guids();
        bool validate_encoder_guid(guid_t g);
        std::string check_error(NVENCSTATUS status);
    };
} 


#endif