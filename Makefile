CXX = g++
CXXFLAGS = -O0 -g -Ilibs/imgui -Ilibs/backends -Ilibs/implot -MMD -MP
LDFLAGS = -lglfw -lGL -lGLEW
TARGET = main
IMGUI_SRC    = $(wildcard libs/imgui/*.cpp)
BACKEND_SRC  = libs/backends/imgui_impl_glfw.cpp libs/backends/imgui_impl_opengl3.cpp
IMPLOT_SRC   = libs/implot/implot.cpp libs/implot/implot_items.cpp
CORE_SRC     = core.cpp
VIEW_SRC     = view.cpp
SRC = $(IMGUI_SRC) $(BACKEND_SRC) $(IMPLOT_SRC) $(CORE_SRC) $(VIEW_SRC)
OBJ = $(patsubst %.cpp, bin/%.o, $(SRC))
DEP = $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

bin/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf bin/* $(TARGET)

-include $(DEP)

.PHONY: all clean
