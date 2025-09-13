CXX := g++

CXXFLAGS := -g -std=c++11 -I. -I./animations

SRCS := $(wildcard *.cpp) $(wildcard animations/*.cpp)

OBJ := $(SRCS:.cpp=.o)
HEADERS := $(wildcard *.h)
SHADERS:= shaders/*.vs shaders/*.fs

LIBS := -lglfw -lGLEW -ldl -lGL $$(pkg-config --cflags --libs libpipewire-0.3) -lm

TARGET := MusicGazer

all: $(TARGET)

$(TARGET): $(OBJ) $(SHADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

install: $(TARGET)
	install -Dm 755 $(TARGET) /usr/bin/$(TARGET)
	install -d /usr/share/$(TARGET)/shaders
	install -m 744 $(SHADERS) -t /usr/share/$(TARGET)/shaders

clean:
	rm -f $(OBJ) $(TARGET)