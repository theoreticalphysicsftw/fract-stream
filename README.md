# fract-stream

This is old project for university course about parallel systems. 

The program generates H.264 stream displaying zooming of a fractal. The fractal images are generated with CUDA and the stream is encoded with NVENC. On the cpu side we have 3 threads, one executing and waiting compute kernels, one submiting rendered frames for encoding and one submiting encoded parts of the stream for serialization. Since the encoding runs on specialized hardware that's independent of the streaming multiprocessors that execute the compute kernels, this work execution strategy saves some time.

The project was supposed to be tested on a server with tesla GPU that's why there's so much double precision stuff.

Unfortunately since I no longer use NVIDIA GPU I can't test if this code even compiles (was this the final version of the project even?).
