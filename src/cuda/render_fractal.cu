#define LN2_INV 1.4426950408889634
#define TWO_PI 6.283185307179586
#define PI 3.141592653589793
#define E 2.718281828459045


__forceinline__ double __device__ exponent(double x)
{
    return exp(x);
}

__forceinline__ float __device__ exponent(float x)
{
    return expf(x);
}

__forceinline__ double __device__ cosine(double x)
{
    return cos(x);
}

__forceinline__ float __device__ cosine(float x)
{
    return cosf(x);
}

__forceinline__ double __device__ sine(double x)
{
    return sin(x);
}

__forceinline__ float __device__ sine(float x)
{
    return sinf(x);
}

template<class T>
__forceinline__ T __device__ clamp(const T& x, const T& lower, const T& upper) 
{
    return min(upper, max(x, lower));
}

template <typename T>
void __device__  render_fractal(unsigned char* nv12_yuv, unsigned int height, unsigned int width, unsigned int stride,
                               T x_0, T x_step, T y_1, T y_step, T cutoff_value_sq, unsigned int max_iterations, 
                               unsigned int x, unsigned int y)
{
    if(x>=width || y>= height)
        return;
        
    unsigned i;

    T initial_real = x_0 + x*x_step;
    T initial_imag = y_1 - y*y_step;
    T current_real = initial_real;
    T current_imag = initial_imag;
    
    T current_abs_sq = current_imag*current_imag+current_real*current_real;
    
    for(i=0; i<max_iterations &&  current_abs_sq < cutoff_value_sq;++i)
    {
        T current_exp_arg = exponent(-current_real);
        T current_square_imag = 2 * current_real * current_imag;
        T cos_current_imag = cosine(current_imag);
        T sin_current_imag = sine(current_imag);
        current_real = current_real*current_real - current_imag*current_imag + current_exp_arg*(cos_current_imag*initial_real + sin_current_imag*initial_imag);
        current_imag = current_exp_arg*(cos_current_imag*initial_imag - sin_current_imag*initial_real) + current_square_imag;
        current_abs_sq = current_imag*current_imag+current_real*current_real;
    }
    
    float luma,cr,cb;
    /* ---- Colorization -----*/
    if(i<max_iterations)
    {
        double ln_cutoff_val_sq = logf(cutoff_value_sq);
        double smoothing_factor = LN2_INV*(log(0.5*log(current_abs_sq)-0.5*log(ln_cutoff_val_sq)));
        double smoothed_iterations = i+1-smoothing_factor;
        // double variable = log(smoothed_iterations*((E-1)/(max_iterations+1-smoothing_factor))+1)*TWO_PI;
        double variable = smoothed_iterations/max_iterations*PI;
        
        luma = 50 + (unsigned char)clamp((200.0 * variable),0.0,200.0);
        cr = (unsigned char)clamp((235.5*cosine(0.5*variable)),16.0,235.0);
        cb = (unsigned char)clamp((235.5*sine(3.0*variable)),16.0,128.0);
    }
    else
    {
        luma = 16;
        cr = 128;
        cb = 128;
    }

    nv12_yuv[y*stride+x] = luma;
    
    if(((x & 1u) == 0u) && ((y & 1u) == 0u))
    {        
        nv12_yuv[height*stride+y*stride/2+x] = cr;
        nv12_yuv[height*stride+y*stride/2+x+1]= cb;
    }
    
}

extern "C" __global__ void render_fractal_float(unsigned char* nv12_yuv,unsigned int height, unsigned int width, unsigned int stride,
                                                float x_0, float x_step, float y_1, float y_step,float cutoff_value_sq,
                                                unsigned int max_iterations)
{
    unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;
    
    render_fractal(nv12_yuv,height,width,stride,x_0,x_step,y_1,y_step,cutoff_value_sq,max_iterations,x,y);
}

extern "C" __global__ void render_fractal_double(unsigned char* nv12_yuv, unsigned int height, unsigned int width, unsigned int stride,
                                                 double x_0, double x_step, double y_1, double y_step, double cutoff_value_sq,
                                                 unsigned int max_iterations)
{
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;
    
    render_fractal(nv12_yuv,height,width,stride,x_0,x_step,y_1,y_step,cutoff_value_sq,max_iterations,x,y);
}

