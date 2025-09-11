CXX := g++

CXXFLAGS := -g -std=c++11

SRCS := $(wildcard *.cpp)
OBJ := $(SRCS:.cpp=.o)
SHADERS:= $(wildcard *.vs) $(wildcard *.fs)

LIBS := -lglfw -lGLEW -ldl -lGL $$(pkg-config --cflags --libs libpipewire-0.3) -lm

TARGET := MusicGazer

all: $(TARGET)

$(TARGET): $(OBJ) $(SHADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

clean:
	rm -f $(OBJ) $(TARGET)