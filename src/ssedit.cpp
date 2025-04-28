#include <math.h>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "log.hpp"

struct Line {
    ImVec2 p1, p2;
    ImU32 color;
    float thickness;
};
std::vector<Line> lines;

static void glfw_error_callback(int error, const char *description) {
    LogPrint(ERR, "GLFW error %d: %s", error, description);
}

GLuint LoadTexture(const char* path, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

void DrawCanvas(GLuint texture, int img_w, int img_h) {
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 content_region = ImGui::GetContentRegionAvail();

    // Compute scale to fit image while preserving aspect ratio
    float scale = std::min(content_region.x / img_w, content_region.y / img_h);
    ImVec2 display_size = ImVec2(img_w * scale, img_h * scale);

    // Center the image inside the content region (optional)
    ImVec2 offset = ImVec2(
        (content_region.x - display_size.x) * 0.5f,
        (content_region.y - display_size.y) * 0.5f
    );
    ImVec2 draw_pos = canvas_pos + offset;

    ImGui::SetCursorScreenPos(draw_pos);
    ImGui::Image((intptr_t)texture, display_size);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    static bool drawing = false;
    static ImVec2 start_pos;

    if (ImGui::IsItemHovered()) {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        ImVec2 local_mouse = (mouse - draw_pos) * (1.0f / scale);  // convert to image space

        if (ImGui::IsMouseClicked(0)) {
            drawing = true;
            start_pos = local_mouse;
        } else if (ImGui::IsMouseReleased(0) && drawing) {
            ImVec2 end_pos = local_mouse;
            lines.push_back({start_pos, end_pos, IM_COL32(255, 0, 0, 255), 2.0f});
            drawing = false;
        }
    }

    for (const auto& line : lines) {
        draw_list->AddLine(draw_pos + line.p1 * scale, draw_pos + line.p2 * scale,
                           line.color, line.thickness * scale);
    }
}

void SaveImage(int width, int height, GLuint image_tex) {
    GLuint fbo, color_tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render the image quad
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, image_tex);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-1, -1);
    glTexCoord2f(1, 1); glVertex2f( 1, -1);
    glTexCoord2f(1, 0); glVertex2f( 1,  1);
    glTexCoord2f(0, 0); glVertex2f(-1,  1);
    glEnd();

    // Draw lines using OpenGL directly
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    for (const auto& line : lines) {
        glVertex2f((line.p1.x / width) * 2.0f - 1.0f, 1.0f - (line.p1.y / height) * 2.0f);
        glVertex2f((line.p2.x / width) * 2.0f - 1.0f, 1.0f - (line.p2.y / height) * 2.0f);
    }
    glEnd();

    std::vector<unsigned char> pixels(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(true);
    stbi_write_png("output.png", width, height, 4, pixels.data(), width * 4);

    glDeleteTextures(1, &color_tex);
    glDeleteFramebuffers(1, &fbo);
}

// Main code
int main(int argc, char **argv) {
    LogInit(INFO, stderr);

    glfwSetErrorCallback(glfw_error_callback);
    if (glfwInit() != GLFW_TRUE) {
        LogPrint(ERR, "failed to initialize glfw!");
        return 1;
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "ssedit", nullptr, nullptr);
    if (window == nullptr) {
        LogPrint(ERR, "failed to create glfw window!");
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        LogPrint(WARN, "GLEW init returned %d (%s)", err, glewGetErrorString(err));
        LogPrint(WARN, "This appears to be a bug: https://github.com/nigels-com/glew/issues/417");
        //return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // disable automatic .ini file saving

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts (TODO: make configurable)
    const char font[] = "/usr/share/fonts/nerdfonts/JetBrainsMonoNLNerdFont-Medium.ttf";
    io.Fonts->AddFontFromFileTTF(font, 19.f);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    int img_w, img_h;
    GLuint tex = LoadTexture(argv[1], img_w, img_h);

    // Main loop
    bool need_export = false;
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowBgAlpha(1.f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, clear_color);
        ImGui::Begin("ssedit", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoSavedSettings);

        ImGui::Text("This is some useful text.");

        ImGui::ColorEdit4("clear color", (float *)&clear_color);

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        if (ImGui::Button("Export to PNG")) {
            need_export = true;
        }

        DrawCanvas(tex, img_w, img_h);

        ImGui::PopStyleColor();
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (need_export) {
            need_export = false;
            SaveImage(img_w, img_h, tex);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

