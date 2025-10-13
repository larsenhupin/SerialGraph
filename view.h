#include <stdio.h>
#include "imgui.h"
#include <GL/glew.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "libs/implot/implot.h"
#include <GLFW/glfw3.h>


int initImGUI(GLFWwindow** window, ImFont** pFont);
void cleanIMGUI(GLFWwindow** window);
int renderToolbar(void);
int renderGraphSerial(Serial *serial);
