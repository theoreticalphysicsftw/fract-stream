CXX=g++
INCLUDES=-I. -I /usr/local/cuda/include/ 
CFLAGS=-O3 -std=c++11 -Wall -fno-exceptions 
LDFLAGS=-pthread -ldl -lcuda -s

CPU_SRC=src/utility/shared_lib_loader.cpp \
	src/nvenc/nvenc_encoder.cpp \
	src/nvenc/frame_buffer.cpp \
	src/cuda/cuda_context.cpp \
	src/cuda/cuda_module_loader.cpp \
	src/frame_queue.cpp \
	src/video_renderer.cpp \
	src/frame_renderer.cpp \
	src/cli_parser.cpp

CPU_TARGETS=$(CPU_SRC:.cpp=.o)
CPU_OBJ=$(addprefix obj/,$(notdir $(CPU_TARGETS)))

all: 	main $(CPU_TARGETS) gpu_kernel
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o bin/zad16_fractal obj/main.o $(CPU_OBJ)

main: 	
	$(CXX) $(CFLAGS) $(INCLUDES) -c src/main.cpp -o obj/main.o

$(CPU_TARGETS) : $(CPU_SRC)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $(patsubst %.o,%.cpp,$@) -o obj/$(notdir $@)

gpu_kernel:
	nvcc -arch sm_30 -cubin src/cuda/render_fractal.cu -o bin/render_fractal.cubin

clean:
	rm $(CPU_OBJ) bin/zad16_fractal obj/main.o
