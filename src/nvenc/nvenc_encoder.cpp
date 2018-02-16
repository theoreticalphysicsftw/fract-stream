#include <algorithm>
#include "nvenc_encoder.hpp"
#include <vector>
#include <cstdint>


namespace fractal
{
    bool nvenc_encoder_t::load_library()
    {
        
#if defined(NV_WINDOWS)
#if defined (_WIN64)
        nvenc_dll = utility::load_shared_lib("nvEncodeAPI64.dll");
#else
        nvenc_dll = utility::load_shared_lib("nvEncodeAPI.dll");
#endif
#else
        nvenc_dll = utility::load_shared_lib("libnvidia-encode.so.1");
#endif
        
        nvenc_init_func_t nvEncodeAPICreateInstance=(nvenc_init_func_t) utility::load_symbol(nvenc_dll,"NvEncodeAPICreateInstance");
        
        nvenc_api = (NV_ENCODE_API_FUNCTION_LIST*) malloc(sizeof(NV_ENCODE_API_FUNCTION_LIST));
        memset(nvenc_api, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
        nvenc_api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
        
        // TODO : Add error checking for unsuccessful load.
        return nvEncodeAPICreateInstance(nvenc_api)==NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::create_enc_session(void* cuda_device)
    {
        NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS params;
        memset(&params,0,sizeof(NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS));
        
        params.version=NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
        params.apiVersion=NVENCAPI_VERSION;
        params.reserved=nullptr;
        // Required according to the API Reference
        std::fill(params.reserved2,params.reserved2+64,nullptr);
        params.device=cuda_device;
        params.deviceType=NV_ENC_DEVICE_TYPE_CUDA;
        
        return nvenc_api->nvEncOpenEncodeSessionEx(&params,&encoder)==NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::select_guids()
    {
        encoder_guid=NV_ENC_CODEC_H264_GUID;
        preset_guid=NV_ENC_PRESET_HQ_GUID;
        // TODO : Validate GUIDs
        return validate_encoder_guid(encoder_guid);
    }
    
    
    nvenc_encoder_t::nvenc_encoder_t(void* cuda_device) : frame_time_stamp(0)
    {
        // TODO : Fix up initialization if error checking is enabled
        load_library();
        create_enc_session(cuda_device);
        select_guids();
    }
    
    
    nvenc_encoder_t::~nvenc_encoder_t()
    {
        nvenc_api->nvEncDestroyEncoder(encoder);
        utility::unload_lib(nvenc_dll);
        free(nvenc_api);
    }
    
    
    bool nvenc_encoder_t::configure(const nvenc_encoder_config_t& cfg)
    {
        memset(&init_params,0,sizeof(init_params));
        memset(&nvenc_config,0,sizeof(nvenc_config));
        
        init_params.version = NV_ENC_INITIALIZE_PARAMS_VER;
        nvenc_config.version = NV_ENC_CONFIG_VER;
        
        init_params.encodeGUID = encoder_guid;
        init_params.presetGUID = preset_guid;
        init_params.encodeWidth = cfg.width;
        init_params.encodeHeight = cfg.height;
        init_params.frameRateNum = cfg.fps;
        init_params.frameRateDen = 1;
        init_params.enableEncodeAsync = 0;
        init_params.enablePTD = 1;
        init_params.reportSliceOffsets = 0;
        init_params.enableSubFrameWrite = 0;
        init_params.enableExternalMEHints = 0;
        init_params.enableMEOnlyMode = 0;
        init_params.encodeConfig = &nvenc_config;
        init_params.maxEncodeWidth = cfg.width;
        init_params.maxEncodeHeight = cfg.height;
       
        nvenc_config.profileGUID = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
        nvenc_config.gopLength = cfg.gop_length;
        nvenc_config.frameIntervalP = cfg.number_of_b_frames+1; 
        nvenc_config.monoChromeEncoding = 0;
        nvenc_config.frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
        nvenc_config.mvPrecision = NV_ENC_MV_PRECISION_DEFAULT;
        
        nvenc_config.rcParams.rateControlMode = (cfg.vbr)? NV_ENC_PARAMS_RC_VBR : NV_ENC_PARAMS_RC_CBR;
        nvenc_config.rcParams.averageBitRate = cfg.avg_bitrate;
        nvenc_config.rcParams.maxBitRate = cfg.max_bitrate;
        nvenc_config.rcParams.vbvBufferSize = 0; // Use default value.
        nvenc_config.rcParams.vbvInitialDelay = 0; // Use default value.
        nvenc_config.rcParams.enableMinQP = 0;
        nvenc_config.rcParams.enableMaxQP = 0;
        nvenc_config.rcParams.enableInitialRCQP = 0;
        nvenc_config.rcParams.enableAQ = 0;
        nvenc_config.rcParams.enableExtQPDeltaMap = 0;
        nvenc_config.rcParams.constQP.qpInterP=cfg.initial_qp;
        nvenc_config.rcParams.constQP.qpInterB=cfg.initial_qp;
        nvenc_config.rcParams.constQP.qpIntra=cfg.initial_qp;
        
        // Not supported by Kepler GPUs
        nvenc_config.encodeCodecConfig.h264Config.enableTemporalSVC = 0; 
        nvenc_config.encodeCodecConfig.h264Config.enableStereoMVC = 0;
        // Not supported by Kepler GPUs
        nvenc_config.encodeCodecConfig.h264Config.hierarchicalPFrames = 0;
        // Not supported by Kepler GPUs
        nvenc_config.encodeCodecConfig.h264Config.hierarchicalBFrames = 0;
        nvenc_config.encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 0;
        nvenc_config.encodeCodecConfig.h264Config.outputPictureTimingSEI = 0;
        nvenc_config.encodeCodecConfig.h264Config.outputAUD = 0;
        nvenc_config.encodeCodecConfig.h264Config.disableSPSPPS = 0;
        nvenc_config.encodeCodecConfig.h264Config.outputFramePackingSEI = 0;
        nvenc_config.encodeCodecConfig.h264Config.outputRecoveryPointSEI = 0;
        nvenc_config.encodeCodecConfig.h264Config.enableIntraRefresh = 0;
        nvenc_config.encodeCodecConfig.h264Config.enableConstrainedEncoding = 0;
        nvenc_config.encodeCodecConfig.h264Config.repeatSPSPPS = 0;
        nvenc_config.encodeCodecConfig.h264Config.enableVFR = 0;
        // Not supported by Kepler GPUs
        nvenc_config.encodeCodecConfig.h264Config.qpPrimeYZeroTransformBypassFlag = 0;
        nvenc_config.encodeCodecConfig.h264Config.useConstrainedIntraPred = 0;
        nvenc_config.encodeCodecConfig.h264Config.level = NV_ENC_LEVEL_AUTOSELECT;
        nvenc_config.encodeCodecConfig.h264Config.separateColourPlaneFlag = 0;
        nvenc_config.encodeCodecConfig.h264Config.disableDeblockingFilterIDC = 0;
        nvenc_config.encodeCodecConfig.h264Config.numTemporalLayers = 0;
        nvenc_config.encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_AUTOSELECT;
        nvenc_config.encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
        nvenc_config.encodeCodecConfig.h264Config.bdirectMode = NV_ENC_H264_BDIRECT_MODE_AUTOSELECT;
        nvenc_config.encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CABAC;
        nvenc_config.encodeCodecConfig.h264Config.maxNumRefFrames = 0; // Use default value
        nvenc_config.encodeCodecConfig.h264Config.sliceMode = 0;
        nvenc_config.encodeCodecConfig.h264Config.sliceModeData = 0;
        nvenc_config.encodeCodecConfig.h264Config.ltrNumFrames = 0;
        nvenc_config.encodeCodecConfig.h264Config.ltrTrustMode = 0;
        nvenc_config.encodeCodecConfig.h264Config.chromaFormatIDC = 1; // yv420
        
        auto status = nvenc_api->nvEncInitializeEncoder(encoder,&init_params);
        
        return status == NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::register_cuda_devptr(void* devptr, void*& registered_resource,size_t stride)
    {
         NV_ENC_REGISTER_RESOURCE register_params;
         memset(&register_params,0,sizeof(register_params));
         
         register_params.version = NV_ENC_REGISTER_RESOURCE_VER;
         register_params.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR;
         register_params.width = init_params.encodeWidth;
         register_params.height = init_params.encodeHeight;
         register_params.pitch = stride;
         register_params.subResourceIndex = 0;
         register_params.resourceToRegister = devptr;
         register_params.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12;
             
         auto status = nvenc_api->nvEncRegisterResource(encoder,&register_params);
         
         registered_resource = register_params.registeredResource;
         
         return status == NV_ENC_SUCCESS;
    }
    
    
     bool nvenc_encoder_t::unregister_cuda_devptr(void* resource)
     {
         return nvenc_api->nvEncUnregisterResource(encoder,resource) == NV_ENC_SUCCESS;
     }
     
     
     bool nvenc_encoder_t::create_bitstream_buffer(output_ptr_t& buffer)
     {
         NV_ENC_CREATE_BITSTREAM_BUFFER bitstream_buffer_params;
         memset(&bitstream_buffer_params,0,sizeof(bitstream_buffer_params));
         
         bitstream_buffer_params.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;
         bitstream_buffer_params.size = init_params.encodeHeight*init_params.encodeWidth*3/2;
         bitstream_buffer_params.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
         
         auto status = nvenc_api->nvEncCreateBitstreamBuffer(encoder,&bitstream_buffer_params);
         
         buffer = bitstream_buffer_params.bitstreamBuffer;
         
         return status == NV_ENC_SUCCESS;
     }
    
    
    bool nvenc_encoder_t::destroy_bitstream_buffer(output_ptr_t buffer)
    {
        return nvenc_api->nvEncDestroyBitstreamBuffer(encoder,buffer) == NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::map_input_resource(void* registered_resource, input_ptr_t& input)
    {
        NV_ENC_MAP_INPUT_RESOURCE map_params;
        memset(&map_params,0,sizeof(map_params));
        
        map_params.version = NV_ENC_MAP_INPUT_RESOURCE_VER;
        map_params.registeredResource = registered_resource;
        
        auto status = nvenc_api->nvEncMapInputResource(encoder,&map_params);
        
        input = map_params.mappedResource;
        
        return status == NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::unmap_input_resource(input_ptr_t input)
    {
        return nvenc_api->nvEncUnmapInputResource(encoder,input) == NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::encode_frame(input_ptr_t in_buff,output_ptr_t out_buff,size_t stride, bool eos)
    {
        NV_ENC_PIC_PARAMS pic_params;
        memset(&pic_params,0,sizeof(pic_params));
        
        pic_params.version = NV_ENC_PIC_PARAMS_VER;
        
        if(eos)
        {
            pic_params.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
        }
        
        else
        {
            pic_params.inputWidth = init_params.encodeWidth;
            pic_params.inputHeight = init_params.encodeHeight;
            pic_params.inputPitch = stride;
            pic_params.inputBuffer = in_buff;
            pic_params.outputBitstream = out_buff;
            pic_params.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12;
            pic_params.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
            pic_params.inputTimeStamp = frame_time_stamp++;
        }
        
        auto status = nvenc_api->nvEncEncodePicture(encoder,&pic_params);
        
        return status == NV_ENC_SUCCESS;

    }
    
    
    bool nvenc_encoder_t::lock_bitstream_buffer(output_ptr_t out_buffer, void*& bitstream_cpu_ptr, uint32_t& bitstream_size)
    {
        NV_ENC_LOCK_BITSTREAM lock_bitstream_params;
        memset(&lock_bitstream_params,0,sizeof(lock_bitstream_params));
        
        lock_bitstream_params.version = NV_ENC_LOCK_BITSTREAM_VER;
        lock_bitstream_params.outputBitstream = out_buffer;
        
        auto status = nvenc_api->nvEncLockBitstream(encoder,&lock_bitstream_params);
        
        bitstream_cpu_ptr = lock_bitstream_params.bitstreamBufferPtr;
        bitstream_size = lock_bitstream_params.bitstreamSizeInBytes;
        
        return status == NV_ENC_SUCCESS;
    }
    
    
    bool nvenc_encoder_t::unlock_bitstream_buffer(output_ptr_t out_buffer)
    {
        return nvenc_api->nvEncUnlockBitstream(encoder,out_buffer) == NV_ENC_SUCCESS;
    }
    
    nvenc_encoder_limits_t nvenc_encoder_t::get_limits()
    {
        nvenc_encoder_limits_t limits;
        
        NV_ENC_CAPS_PARAM params;
        // Required by the API
        memset(&params,0,sizeof(NV_ENC_CAPS_PARAM ));
                
        params.version=NV_ENC_CAPS_PARAM_VER;
        
        int value;
        
        params.capsToQuery=NV_ENC_CAPS_NUM_MAX_BFRAMES;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        limits.max_b_frames = value;
        
        params.capsToQuery=NV_ENC_CAPS_WIDTH_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        limits.max_width = value;
        
        params.capsToQuery=NV_ENC_CAPS_HEIGHT_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        limits.max_height = value;
        
        return limits;
    }
    
    std::string nvenc_encoder_t::get_capabilities()
    {
        std::string result;
        static constexpr size_t initial_size=64*1024;
        result.reserve(initial_size);
        
        NV_ENC_CAPS_PARAM params;
        // Required by the API
        memset(&params,0,sizeof(NV_ENC_CAPS_PARAM ));
                
        params.version=NV_ENC_CAPS_PARAM_VER;
        
        int value;
        
        params.capsToQuery=NV_ENC_CAPS_NUM_MAX_BFRAMES;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Maximum B-frames : ";
        result+=std::to_string(value);
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_MONOCHROME;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for monochrome encoding : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_QPELMV;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for Qpel motion estimation : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_BDIRECT_MODE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for BDirect mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_CABAC;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for CABAC : ";
        result+=(value)? "yes" : "no";
        result+="\n";
    
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_ADAPTIVE_TRANSFORM;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for adaptive transform : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_HIERARCHICAL_PFRAMES;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for hierarchical P-frames : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_HIERARCHICAL_PFRAMES;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for hierarchical P-frames : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_HIERARCHICAL_BFRAMES;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for hierarchical B-frames : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SEPARATE_COLOUR_PLANE ;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for separate colour plane encoding : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_WIDTH_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Maximum output width : ";
        result+=std::to_string(value);
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_HEIGHT_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Maximum output height : ";
        result+=std::to_string(value);
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_TEMPORAL_SVC;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for temporal SVC encoding : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYN_RES_CHANGE ;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for dynamic encode resolution change : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYN_BITRATE_CHANGE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for dynamic encode bitrate change : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYN_BITRATE_CHANGE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for dynamic encode bitrate change : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYN_FORCE_CONSTQP;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for forcing constant QP on the fly : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYN_RCMODE_CHANGE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for dynamic rate control mode change : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_CONSTRAINED_ENCODING;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for subframe readback : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_CONSTRAINED_ENCODING;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for constrained encoding mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_INTRA_REFRESH;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for intra refresh mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_CUSTOM_VBV_BUF_SIZE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for custom VBV buffer size : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_DYNAMIC_SLICE_MODE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for dynamic slice mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_REF_PIC_INVALIDATION;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for reference picture invalidation : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_ASYNC_ENCODE_SUPPORT;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Support for async mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_YUV444_ENCODE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for YUV444 frames encoding : ";
        result+=(value)? "yes" : "no";
        result+="\n";

        params.capsToQuery=NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for loseless encoding : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_SAO;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for sample adaptive offset : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_SUPPORT_MEONLY_MODE;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Hardware support for MEonly mode : ";
        result+=(value)? "yes" : "no";
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_MB_NUM_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Maximum MBs per frame : ";
        result+=std::to_string(value);
        result+="\n";
        
        params.capsToQuery=NV_ENC_CAPS_MB_PER_SEC_MAX;
        nvenc_api->nvEncGetEncodeCaps(encoder,encoder_guid,&params,&value);
        result+="Maximum aggregate throughput in MBs per sec : ";
        result+=std::to_string(value);
        result+="\n";

        return result;
    }
    
    
    bool operator==(const GUID& guid1, const GUID& guid2)
    {
        for (auto i=0;i<sizeof(GUID);++i)
            if (((char*) &guid1)[i]!=((char*) &guid2)[i])
                return false;
        return true;
    }
    
    
    bool nvenc_encoder_t::validate_encoder_guid(guid_t g)
    {
        std::vector<guid_t> guids;
        uint32_t guid_count;
        NVENCSTATUS nvenc_status;
        
        nvenc_status = nvenc_api->nvEncGetEncodeGUIDCount(encoder, &guid_count);
        guids.resize(guid_count);
        nvenc_status = nvenc_api->nvEncGetEncodeGUIDs(encoder, guids.data(), guid_count, &guid_count);
        guids.resize(guid_count);
        
        if (nvenc_status != NV_ENC_SUCCESS)
            return false;

        for(auto i=guids.begin();i!=guids.end();++i)
        {
            if (*i==encoder_guid)
                return true;
        }
        
        return false;
    }
    
    
    std::string nvenc_encoder_t::check_error(NVENCSTATUS status)
    {
        switch (status)
        {
            case NV_ENC_SUCCESS : return "Success!";
            case NV_ENC_ERR_NO_ENCODE_DEVICE : return "No encode capable devices were detected!";
            case NV_ENC_ERR_UNSUPPORTED_DEVICE: return "The encoder device supplied by the client is not supported!";
            case NV_ENC_ERR_INVALID_ENCODERDEVICE : return "The encoder device supplied by the client is not valid!";
            case NV_ENC_ERR_INVALID_DEVICE : return "The device passed to the API call is not valid!";
            case NV_ENC_ERR_DEVICE_NOT_EXIST : return "The device passed to the API call is no longer available!";
            case NV_ENC_ERR_INVALID_PTR : return "One or more of the pointers passed to the API call are not valid!";
            case NV_ENC_ERR_INVALID_EVENT : return "The completion event passed in ::NvEncEncodePicture() call is not valid!";
            case NV_ENC_ERR_INVALID_PARAM : return "One or more of the parameters passed to the API call are not valid!";
            case NV_ENC_ERR_INVALID_CALL : return "An API call was made in wrong sequence/order!";
            case NV_ENC_ERR_OUT_OF_MEMORY : return "API call failed because it was unable to allocate enough memory!";
            case NV_ENC_ERR_ENCODER_NOT_INITIALIZED : return "The encoder has not been initialized or initialization has failed!";
            case NV_ENC_ERR_UNSUPPORTED_PARAM : return "An unsupported parameter was passed by the client!";
            case NV_ENC_ERR_LOCK_BUSY : return "The ::NvEncLockBitstream() failed to lock the output buffer.";
            case NV_ENC_ERR_NOT_ENOUGH_BUFFER : return "The size of the user buffer passed by the client is insufficient!";
            case NV_ENC_ERR_INVALID_VERSION : return "An invalid struct version was used by the client!";
            case NV_ENC_ERR_MAP_FAILED : return "NvEncMapInputResource() API failed to map the client provided input resource!";
            case NV_ENC_ERR_NEED_MORE_INPUT : return "Driver requires more input buffers to produce an output bitstream.";
            case NV_ENC_ERR_ENCODER_BUSY : return "The HW encoder is busy encoding and is unable to encode the input!";
            case NV_ENC_ERR_EVENT_NOT_REGISTERD : return "The completion event passed in ::NvEncEncodePicture() has not been registered!";
            case NV_ENC_ERR_GENERIC : return "An unknown internal error has occurred!";
            case NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY : return "The client is attempting to use a feature that is not available!";
            case NV_ENC_ERR_UNIMPLEMENTED : return  "The client is attempting to use a feature that is not implemented!";
            case NV_ENC_ERR_RESOURCE_REGISTER_FAILED : return "The ::NvEncRegisterResource API failed to register the resource!";
            case NV_ENC_ERR_RESOURCE_NOT_REGISTERED : return "The client is attempting to unregister a resource that has not been registered!";
            case NV_ENC_ERR_RESOURCE_NOT_MAPPED : return "The client is attempting to unmap a resource that has not been mapped!";
            default : return "Unknown external error has occured!";
        }
    }
}

