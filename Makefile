CXX := g++
CC := gcc

CXXFLAGS := -g -std=c++17 -Wall -I. -I./animations -I./glad/include
CFLAGS := -g -Wall -I. -I./animations -I./glad/include

CPP_SRCS := $(wildcard *.cpp) $(wildcard animations/*.cpp)
C_SRCS := glad/src/glad.c

OBJ := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
HEADERS := $(wildcard *.h)
SHADERS:= shaders/*.vs shaders/*.fs

LIBS := -lglfw -ldl -lGL $$(pkg-config --cflags --libs libpipewire-0.3) -lm

TARGET := MusicGazer

all: $(TARGET)

$(TARGET): $(OBJ) $(SHADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -Dm 755 $(TARGET) /usr/bin/$(TARGET)
	install -d /usr/share/$(TARGET)/shaders
	install -m 744 $(SHADERS) -t /usr/share/$(TARGET)/shaders

clean:
	rm -f $(OBJ) $(TARGET)