#include "core.h"
#include "view.h"

int main() {
    // Create Window
    GLFWwindow* window = NULL;
    ImFont* pFont = NULL;
    int init = initImGUI(&window, &pFont);

    // Open serial and setup termios
    int fileDescriptor = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    configureTermios(&fileDescriptor);

    // Setup serial
    Serial serial;
    setupSerial(&serial);

    // Main loop
    while(!glfwWindowShouldClose(window)) {
        // Data
        readSerialLineRaw(&serial, fileDescriptor);

        // Events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Window title
        char title[128];
        snprintf(title, sizeof(title), "Serial - FPS: %.1f", ImGui::GetIO().Framerate);
        glfwSetWindowTitle(window, title);

        // Body
        if (!renderToolbar()) printf("Error: renderToolbar failed");
        if (!renderGraphSerial(&serial)) printf("Error: renderGraphSerial failed");

        // Rendering
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Free memory
    cleanSerial(&serial);
    close(fileDescriptor);
    cleanIMGUI(&window);

    return 0;
}

int renderToolbar(void) {
    ImGui::SetNextWindowPos(ImVec2(0, 26), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 70), ImGuiCond_Always);
    ImGui::Begin(" ", NULL,
            ImGuiWindowFlags_NoMove       |
            ImGuiWindowFlags_NoCollapse   |
            ImGuiWindowFlags_NoResize);

    ImGui::End();

    return 1;
}

int renderGraphSerial(Serial *serial) {

    for (int i = 0; i < serial->capacity; i++) {
        int index = (serial->head - serial->capacity + i + serial->capacity) % serial->capacity;
        serial->xAxisData[i] = i;
        serial->yAxisData[i]  = strtof(serial->buffer[index][0], NULL);
    }

    ImGui::SetNextWindowPos(ImVec2(0, 70), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(
                                    ImGui::GetIO().DisplaySize.x-0,
                                    ImGui::GetIO().DisplaySize.y-70),
                                    ImGuiCond_Always);
    ImGui::Begin("plot_container", NULL,
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoTitleBar);

    double y_min = 1000;
    double y_max = 2600.0;

    if (ImPlot::BeginPlot("Graph", ImVec2(-1, ImGui::GetContentRegionAvail().y))) {
        ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
        ImPlot::PlotLine("ac0 (mv)", serial->xAxisData, serial->yAxisData, serial->capacity);
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }

    ImGui::End();
    return 1;
}


int initImGUI(GLFWwindow** window, ImFont** pFont) {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: Failed to initialize GLFW \n");
	    return -1;
    }
   
    // Create a window
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    *window = glfwCreateWindow(mode->width, mode->height, "Sparkland", NULL, NULL);
    glfwSetWindowAttrib(*window, GLFW_DECORATED, GLFW_TRUE);
    glfwSetWindowPos(*window, 0, 0);

    if (!*window) {
        glfwTerminate();
	    return -1;
    }

    glfwMakeContextCurrent(*window);

    // Initialize OpenGL loader
    if (glewInit() != GLEW_OK) {
    	fprintf(stderr, "ERROR: Failed to initialize OpenGL loader (GLEW) \n");
	    return -1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    *pFont = io.Fonts->AddFontFromFileTTF("libs/asset/font/Roboto-Regular.ttf", 18.0f);
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(*window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!ImPlot::CreateContext()) {
        return -1;
    }

    return 1;
}

void cleanIMGUI(GLFWwindow** window) {

    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(*window);
    glfwTerminate();
}
