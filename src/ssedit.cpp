#include <clocale>
#include <cmath>
#include <vector>
#include <memory>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "shapes.hpp"
#include "log.hpp"

#define IMVEC4_TO_COL32(vec) (IM_COL32(vec.x * 255, vec.y * 255, vec.z * 255, vec.w * 255))

std::vector<std::unique_ptr<Shape>> shapes;

struct RenderContext {
    GLuint color_tex;
    GLuint fbo;
    ImGuiContext* imgui_context;
};

enum Tool {
    LINE,
    CIRCLE,
    RECTANGLE,
    FREEFORM,
    ARROW,
};

RenderContext create_render_context(int w, int h) {
    RenderContext ctx;

    // Create FBO and texture
    glGenTextures(1, &ctx.color_tex);
    glBindTexture(GL_TEXTURE_2D, ctx.color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &ctx.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ctx.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.color_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LogPrint(ERR, "Framebuffer not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return ctx;
}

static void glfw_error_callback(int error, const char *description) {
    LogPrint(ERR, "GLFW error %d: %s", error, description);
}

static void glfw_char_callback(GLFWwindow *window, uint32_t codepoint) {
    LogPrint(INFO, "GLFW char: %lc", (wchar_t)codepoint);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Check if Ctrl is held down
    bool ctrlPressed = (mods & GLFW_MOD_CONTROL) || (mods & GLFW_MOD_SUPER);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (ctrlPressed) {
            switch (key) {
            case GLFW_KEY_C:
                // Handle Ctrl+C
                LogPrint(INFO, "GLFW key: Ctrl+C pressed");
                // Your copy logic here
                break;

            case GLFW_KEY_V:
                // Handle Ctrl+V
                LogPrint(INFO, "GLFW key: Ctrl+V pressed");
                // Your paste logic here
                break;

            case GLFW_KEY_Z:
                // Handle Ctrl+Z
                LogPrint(INFO, "GLFW key: Ctrl+Z pressed");
                // Your undo logic here
                break;
            }
        }
    }
}

// In your initialization code:
void SaveImage(int width, int height, void *data) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hide window since we render offscreen
    GLFWwindow* window2 = glfwCreateWindow(width, height, "Offscreen ImGui", nullptr, nullptr);
    if (!window2) {
        LogPrint(ERR, "Failed to create window");
        return;
    }

    glfwMakeContextCurrent(window2);
    glewExperimental = true;
    glewInit();

    RenderContext ctx2 = create_render_context(width, height);

    // Setup second ImGui context
    ctx2.imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx2.imgui_context);
    ImGuiIO& io2 = ImGui::GetIO(); (void)io2;
    ImGui_ImplGlfw_InitForOpenGL(window2, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    GLuint fbo, color_tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

    GLuint image_tex;
    glGenTextures(1, &image_tex);
    glBindTexture(GL_TEXTURE_2D, image_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowBgAlpha(1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Offscreen Window", nullptr, ImGuiWindowFlags_NoMove
                                              | ImGuiWindowFlags_NoDecoration
                                              | ImGuiWindowFlags_NoSavedSettings);

    ImGui::Image(image_tex, ImVec2(width, height));

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    for (const auto &shape: shapes) {
        shape->Draw(draw_list, ImVec2(0, 0), 1);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    std::vector<unsigned char> pixels(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(true);
    stbi_write_png("output.png", width, height, 4, pixels.data(), width * 4);

    glDeleteTextures(1, &color_tex);
    glDeleteFramebuffers(1, &fbo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(ctx2.imgui_context);
    glfwDestroyWindow(window2);
}

// Main code
int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
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
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
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

    int img_w, img_h;
    int channels;
    unsigned char *data = stbi_load(argv[1], &img_w, &img_h, &channels, 4);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Main loop
    bool need_export = false;

    enum Tool active_tool = FREEFORM;
    float thickness = 2.0f;
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool drawing_active = false;
    ImVec2 drawing_start_pos = ImVec2(0, 0);
    std::unique_ptr<Shape> drawing_shape = NULL;

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        //ImGui::SetNextWindowBgAlpha(1.f);
        //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
        ImGui::Begin("ssedit", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoSavedSettings);

        ImGui::ColorEdit4("Color", (float *)&color);

        ImGui::SliderFloat("Thickness", &thickness, 1.0f, 30.0f);

        if (ImGui::RadioButton("Line", active_tool == LINE)) { active_tool = LINE; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Circle", active_tool == CIRCLE)) { active_tool = CIRCLE; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rect", active_tool == RECTANGLE)) { active_tool = RECTANGLE; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Freeform", active_tool == FREEFORM)) { active_tool = FREEFORM; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Arrow", active_tool == ARROW)) { active_tool = ARROW; }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        if (ImGui::Button("Export to PNG")) {
            need_export = true;
        }

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
        ImGui::Image((intptr_t)tex, display_size);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        for (const auto &shape: shapes) {
            shape->Draw(draw_list, draw_pos, scale);
        }
        if (drawing_shape) {
            drawing_shape->Draw(draw_list, draw_pos, scale);
        }

        if (ImGui::IsItemHovered()) {
            ImVec2 mouse = ImGui::GetIO().MousePos;
            ImVec2 local_mouse = (mouse - draw_pos) * (1.0f / scale);  // convert to image space

            if (ImGui::IsMouseClicked(0)) {
                drawing_active = true;
                drawing_start_pos = local_mouse;

                switch (active_tool) {
                case LINE:
                    drawing_shape = std::make_unique<Line>(drawing_start_pos,
                                                           IMVEC4_TO_COL32(color),
                                                           thickness);
                    break;
                case CIRCLE:
                    drawing_shape = std::make_unique<Circle>(drawing_start_pos,
                                                             IMVEC4_TO_COL32(color),
                                                             thickness);
                    break;
                case RECTANGLE:
                    drawing_shape = std::make_unique<Rectangle>(drawing_start_pos,
                                                                IMVEC4_TO_COL32(color),
                                                                thickness);
                    break;
                case FREEFORM:
                    drawing_shape = std::make_unique<Freeform>(drawing_start_pos,
                                                               IMVEC4_TO_COL32(color),
                                                               thickness);
                    break;
                case ARROW:
                    drawing_shape = std::make_unique<Arrow>(drawing_start_pos,
                                                            IMVEC4_TO_COL32(color),
                                                            thickness);
                    break;
                }
            } else if (drawing_active) {
                drawing_shape->Update(local_mouse);

                if (ImGui::IsMouseReleased(0)) {
                    shapes.push_back(std::move(drawing_shape));
                    drawing_shape.reset();
                    drawing_active = false;
                    drawing_start_pos = ImVec2(0, 0);
                }
            }
        }

        //ImGui::PopStyleColor();
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (need_export) {
            ImGuiContext *ctx = ImGui::GetCurrentContext();
            need_export = false;
            SaveImage(img_w, img_h, data);
            glfwMakeContextCurrent(window);
            ImGui::SetCurrentContext(ctx);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Delete OpenGL texture
    glDeleteTextures(1, &tex);

    // Free image memory
    stbi_image_free(data);

    // Destroy window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

