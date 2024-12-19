#include "../include/renderEngine.h"
#include "SDL_stdinc.h"

#include <vector>

#if defined(_WIN64)
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_opengl.h>
#include <SDL_video.h>

#include <cstddef>
#include <cstdio>
#include <iostream>

#include "../depend/imgui/backends/imgui_impl_opengl3.h"
#include "../depend/imgui/backends/imgui_impl_sdl2.h"
#include "../depend/imgui/imgui.h"
#include "../depend/implot/implot.h"
#include "../include/PIDController.h"
#include "../include/core.h"

renderEngine::renderEngine() { }
renderEngine::~renderEngine() { }
SDL_Window* window;
SDL_GLContext gl_context;
ImGuiIO io;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Draw references
std::vector<CircleData>* reactorMaterialRef;
std::vector<CircleData>* neturonRef;
std::vector<RectangleData>* waterRef;
std::vector<RectangleData>* rodRef;

// Simple control rod settings
bool global = true;
bool automode = false;
int automode_goal = 30;
int automode_maxheight = 30;
float automode_speed = 5;

// Advanced control rod settings
bool useController = false;
PID controller;
float controller_Kp = 0.5;
float controller_Ki = 0.1;
float controller_Kd = 1;

// Linking render data
void renderEngine::LinkReactorMaterials(std::vector<CircleData>* newPos)
{
    reactorMaterialRef = newPos;
}
void renderEngine::LinkNeutrons(std::vector<CircleData>* newPos)
{
    neturonRef = newPos;
}
void renderEngine::LinkReactorWater(std::vector<RectangleData>* newPos)
{
    waterRef = newPos;
}
void renderEngine::LinkReactorRod(std::vector<RectangleData>* newPos)
{
    rodRef = newPos;
}

// Start engine
void renderEngine::Initialise(const char* title, int w, int h)
{
    // SDL Attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window with flags
    Uint32 window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL) | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, w, h, window_flags);

    // Initialise renderer
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = ImGui::GetIO();
    (void)io;
    // ImGui::GetIO().IniFilename = NULL;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);

    // Setup Done
    isRunning = true;
}

// Start (AFter Init, Before Update)
void renderEngine::Start()
{
    // PID
    controller = PID(controller_Kp, controller_Ki, controller_Kd, automode_goal, 0, 100);
}

// Tick renderengine
void renderEngine::Update()
{
    // Tick
    tick++;

    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            isRunning = false;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            isRunning = false;
    }

    //  Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Menu Bar
    ImGui::BeginMainMenuBar();
    ImGui::Text("NIP-Engine");
    ImGui::Separator();
    ImGui::Text(NE_VERSION);
    ImGui::Separator();
    ImGui::EndMainMenuBar();

    // Toolbox
    ImGui::Begin("Toolbox", NULL,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::SliderInt("Fission Neutron Count", &settings->fissionNeutronCount, 1, 5);
    ImGui::SliderFloat("Fission Neutron Speed", &settings->fissionNeutronSpeed, 0, 1000);
    ImGui::SliderFloat("Decay Chance", &settings->decayChance, 0, 0.5);
    ImGui::SliderFloat("Neutron Absorption Chance", &settings->waterAbsorptionChance, 0, 0.5);
    ImGui::SliderFloat("Xenon Decay Chance", &settings->xenonDecayChance, 0, 0.5);
    ImGui::SliderFloat("Dissipate Speed", &settings->heatDissipate, 0, 100);
    ImGui::SliderFloat("Heat Transfer Speed", &settings->heatTransfer, 0, 100);
    ImGui::SliderFloat("Water Flow Rate", &settings->waterFlow, 0, 100);
    ImGui::End();

    // Control rod Manager
    ImGui::Begin("Control Rod Manager", NULL,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Automatic", &automode);
    ImGui::Checkbox("Global", &global);
    ImGui::Checkbox("Use PID", &useController);
    ImGui::BeginDisabled(!global || automode);
    ImGui::SliderFloat("Rod Insertion", &settings->rodHeight_1, 1, 100);
    ImGui::EndDisabled();
    ImGui::Separator();
    ImGui::BeginDisabled(global || automode);
    ImGui::SliderFloat("Rod 1 Insertion", &settings->rodHeight_1, 1, 100);
    ImGui::SliderFloat("Rod 2 Insertion", &settings->rodHeight_2, 1, 100);
    ImGui::SliderFloat("Rod 3 Insertion", &settings->rodHeight_3, 1, 100);
    ImGui::SliderFloat("Rod 4 Insertion", &settings->rodHeight_4, 1, 100);
    ImGui::SliderFloat("Rod 5 Insertion", &settings->rodHeight_5, 1, 100);
    ImGui::EndDisabled();
    ImGui::Separator();
    ImGui::BeginDisabled(!automode);
    ImGui::SliderInt("Min Rod Height", &automode_maxheight, 0, 50);
    ImGui::SliderFloat("Rod Speed", &automode_speed, 0, 10);
    ImGui::SliderInt("Reactivity Goal", &automode_goal, 1, 500);
    ImGui::EndDisabled();
    ImGui::Separator();
    ImGui::BeginDisabled(!useController);
    ImGui::SliderFloat("P", &controller_Kp, -3, 3);
    ImGui::SliderFloat("I", &controller_Ki, -3, 3);
    ImGui::SliderFloat("D", &controller_Kd, -3, 3);
    ImGui::EndDisabled();
    ImGui::End();

    if (automode) {
        global = true;
        if (useController) {
            float signal = controller.Calculate(*neutronCount, NE_DELTATIME, controller_Kp, controller_Ki, controller_Kd);
            settings->rodHeight_1 -= signal * NE_DELTATIME;
        } else {
            // Use basic automode
            if (*neutronCount < automode_goal) {

                settings->rodHeight_1 -= automode_speed * NE_DELTATIME;
            } else if (*neutronCount > automode_goal) {
                settings->rodHeight_1 += automode_speed * NE_DELTATIME;
            }

            if (settings->rodHeight_1 > 100) {
                settings->rodHeight_1 = 100;
            } else if (settings->rodHeight_1 < automode_maxheight) {
                settings->rodHeight_1 = automode_maxheight;
            }
        }
    }
    // Set all rods
    if (global) {
        settings->rodHeight_2 = settings->rodHeight_1;
        settings->rodHeight_3 = settings->rodHeight_1;
        settings->rodHeight_4 = settings->rodHeight_1;
        settings->rodHeight_5 = settings->rodHeight_1;
    }

    // Neutron Summoner
    ImGui::Begin("Neutron Summoner", NULL,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    addNeutrons = 0;
    ImGui::SameLine();
    if (ImGui::Button("Add 1x")) {
        addNeutrons = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add 10x")) {
        addNeutrons = 10;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add 100x")) {
        addNeutrons = 100;
    }
    ImGui::SameLine();
    clearAllNeutrons = ImGui::Button("Clear");
    ImGui::End();

    // Top left Overlay //TODO
    /*  if (currentDebugInfo.size() > 0) {
         ImGui::SetNextWindowBgAlpha(0.35f);
         const float PAD = 10.0f;
         const ImGuiViewport* viewport = ImGui::GetMainViewport();
         ImVec2 work_pos = viewport->WorkPos;
         ImVec2 work_size = viewport->WorkSize;
         ImVec2 window_pos, window_pos_pivot;
         window_pos.x = (work_pos.x + 10);
         window_pos.y = (work_pos.y + 10);
         window_pos_pivot.x = 0.0f;
         window_pos_pivot.y = 0.0f;
         ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
         ImGui::Begin("Debug", NULL,
             ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
         ImGui::Text("Debug");
         for (int i = 0; i < currentDebugInfo.size(); i++) {
             ImGui::Text("%s", currentDebugInfo[i].c_str());
         }
         ImGui::End();
     } */

    // Primary Renderer
    ImGui::Begin("Nuclear Reactor", NULL,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(p.x, p.y), ImVec2(p.x + (NR_SIZE_X * RR_SCALE), p.y + (NR_SIZE_Y * RR_SCALE)),
        IM_COL32(255, 255, 255, 255));

    // Draw Reactor Water
    for (int i = 0; i < waterRef->size(); i++) {
        auto col = IM_COL32((*waterRef)[i].colourID, 20, 255 - (*waterRef)[i].colourID, 255);
        if ((*waterRef)[i].colourID == -1) {
            col = IM_COL32(0, 0, 0, 0);
        }
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(p.x + ((*waterRef)[i].position.x - ((*waterRef)[i].size.x / 1)), p.y + ((*waterRef)[i].position.y - ((*waterRef)[i].size.y / 1))), ImVec2(p.x + ((*waterRef)[i].position.x + ((*waterRef)[i].size.x / 1)), p.y + ((*waterRef)[i].position.y + ((*waterRef)[i].size.y / 1))),
            col);
    }

    // Draw Reactor Materials
    for (int i = 0; i < reactorMaterialRef->size(); i++) {
        auto col = IM_COL32(0, 0, 0, 255);
        if ((*reactorMaterialRef)[i].colourID == 0) {
            col = IM_COL32(200, 200, 200, 255);
        } else if ((*reactorMaterialRef)[i].colourID == 1) {
            col = IM_COL32(100, 200, 100, 255);
        } else if ((*reactorMaterialRef)[i].colourID == 2) {
            col = IM_COL32(50, 50, 50, 255);
        }
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(p.x + (*reactorMaterialRef)[i].position.x,
                p.y + (*reactorMaterialRef)[i].position.y),
            (*reactorMaterialRef)[i].radius, col, 0);
    }

    // Draw Neutrons
    for (int i = 0; i < neturonRef->size(); i++) {
        auto col = IM_COL32(50, 50, 50, 255);
        if ((*neturonRef)[i].colourID == 1) {
            col = IM_COL32(100, 100, 100, 255);
        }
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(p.x + (*neturonRef)[i].position.x,
                p.y + (*neturonRef)[i].position.y),
            (*neturonRef)[i].radius, col, 0);
    }

    // Draw Reactor rods
    for (int i = 0; i < rodRef->size(); i++) {
        auto col = IM_COL32(50, 50, 50, 255);
        if ((*rodRef)[i].colourID == 1) {
            col = IM_COL32(100, 100, 100, 255);
        }
        if ((*rodRef)[i].colourID == -1) {
            col = IM_COL32(200, 200, 200, 255);
        }
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(p.x + ((*rodRef)[i].position.x - ((*rodRef)[i].size.x / 2)), p.y + ((*rodRef)[i].position.y - ((*rodRef)[i].size.y / 2))), ImVec2(p.x + ((*rodRef)[i].position.x + ((*rodRef)[i].size.x / 2)), p.y + ((*rodRef)[i].position.y + ((*rodRef)[i].size.y / 2))),
            col);
    }
    ImGui::Dummy(ImVec2((NR_SIZE_X * RR_SCALE), (NR_SIZE_Y * RR_SCALE)));
    ImGui::End();

    // Data Output
    ImGui::Begin("Data", NULL);
    if (ImPlot::BeginPlot("Data Output")) {
        // float time[settings->particle.GetMax()];
        static std::vector<float> time(settings->stats.GetMax());
        for (int i = 0; i < settings->stats.GetMax(); i++) {
            time[i] = i;
        }
        static std::vector<float> data(settings->stats.GetMax());
        for (int i = 0; i < settings->stats.GetMax(); i++) {
            data[i] = settings->stats.GetReactivityStats()[i];
        }
        static std::vector<float> data1(settings->stats.GetMax());
        for (int i = 0; i < settings->stats.GetMax(); i++) {
            data1[i] = settings->stats.GetXenonStats()[i];
        }
        static std::vector<float> data2(settings->stats.GetMax());
        for (int i = 0; i < settings->stats.GetMax(); i++) {
            data2[i] = settings->stats.GetTempStats()[i];
        }

        ImPlot::PlotLine("Reactivity", &time[0], &data[0],
            settings->stats.GetMax());
        ImPlot::PlotLine("Xenon", &time[0], &data1[0],
            settings->stats.GetMax());
        ImPlot::PlotLine("Average Temperature", &time[0], &data2[0],
            settings->stats.GetMax());
        ImPlot::EndPlot();
    }
    ImGui::End();
}

// Render
void renderEngine::Render()
{
    // Imgui Render
    ImGui::Render();

    // Clear and render
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

// Clean
void renderEngine::Clean()
{
    // Shutdown imgui graphic implementation
    ImGui_ImplOpenGL3_Shutdown();
    // Clean Imgui
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    // Clean SDL
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Engine Cleaned!" << std::endl;
}
