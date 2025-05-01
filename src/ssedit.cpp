#include <vector>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <cmath>
#include <fcntl.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shapes.hpp"
#include "utils.hpp"
#include "decode.hpp"
#include "encode.hpp"
#include "clibpoard.hpp"
#include "features.hpp"
#include "icons.hpp"
#include "config.hpp"
#include "log.hpp"

#define STREQ(a, b) (strcmp((a), (b)) == 0)

#define IMVEC4_TO_COL32(vec) (IM_COL32(vec.x * 255, vec.y * 255, vec.z * 255, vec.w * 255))

enum Tool {
    LINE,
    CIRCLE,
    RECTANGLE,
    FREEFORM,
    ARROW,
};

static const char glsl_version[] = "#version 130";

std::vector<std::unique_ptr<Shape>> redo_list;
std::vector<std::unique_ptr<Shape>> shapes;

bool Undo(void) {
    if (!shapes.empty()) {
        redo_list.push_back(std::move(shapes.back()));
        shapes.pop_back();
        return true;
    }
    return false;
}

bool Redo(void) {
    if (!redo_list.empty()) {
        shapes.push_back(std::move(redo_list.back()));
        redo_list.pop_back();
        return true;
    }
    return false;
}

static void glfw_error_callback(int error, const char *description) {
    LogPrint(ERR, "GLFW error %d: %s", error, description);
}

static void glfw_char_callback(GLFWwindow *window, uint32_t codepoint) {
    LogPrint(INFO, "GLFW char: %lc", (wchar_t)codepoint);
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    bool ctrl = mods & GLFW_MOD_CONTROL;
    bool shift = mods & GLFW_MOD_SHIFT;

    switch (key) {
    case GLFW_KEY_C:
        LogPrint(INFO, "GLFW key: Ctrl+C pressed");
        break;
    case GLFW_KEY_V:
        LogPrint(INFO, "GLFW key: Ctrl+V pressed");
        break;
    case GLFW_KEY_Z:
        LogPrint(INFO, "GLFW key: %s%sZ pressed", ctrl ? "Ctrl" : "", shift ? "Shift" : "");
        if (ctrl && shift) {
            Redo();
        } else if (ctrl) {
            Undo();
        }
        break;
    }
}

Image *GetModifiedPixels(Image *orig_image) {
    Image *raw_image;
    unsigned char *pixels_buf = nullptr;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hide window since we render offscreen
    GLFWwindow *window2 = glfwCreateWindow(orig_image->w, orig_image->h,
                                           "ssedit_offscr", nullptr, nullptr);
    if (!window2) {
        LogPrint(ERR, "Failed to create window");
        return nullptr;
    }
    glfwMakeContextCurrent(window2);

    // Setup second ImGui context
    ImGuiContext *imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // disable automatic .ini file saving
    ImGui_ImplGlfw_InitForOpenGL(window2, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint color_tex;
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orig_image->w, orig_image->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

    GLuint image_tex;
    glGenTextures(1, &image_tex);
    glBindTexture(GL_TEXTURE_2D, image_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orig_image->w, orig_image->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, orig_image->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0, 0, orig_image->w, orig_image->h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(orig_image->w, orig_image->h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Offscreen Window", nullptr, ImGuiWindowFlags_NoMove
                                              | ImGuiWindowFlags_NoDecoration
                                              | ImGuiWindowFlags_NoSavedSettings);

    ImGui::Image(image_tex, ImVec2(orig_image->w, orig_image->h));

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    for (const auto &shape: shapes) {
        shape->Draw(draw_list, ImVec2(0, 0), 1);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    pixels_buf = (unsigned char *)malloc(orig_image->data_size);
    glReadPixels(0, 0, orig_image->w, orig_image->h, GL_RGBA, GL_UNSIGNED_BYTE, pixels_buf);
    for (uint32_t y = 0; y < orig_image->h / 2; ++y) {
        int top_index = y * orig_image->w * 4;
        int bottom_index = (orig_image->h - 1 - y) * orig_image->w * 4;
        for (uint32_t x = 0; x < orig_image->w * 4; ++x) {
            std::swap(pixels_buf[top_index + x], pixels_buf[bottom_index + x]);
        }
    }
    raw_image = new Image(pixels_buf, orig_image->data_size,
                          orig_image->w, orig_image->h, Format::RGBA);

    glDeleteTextures(1, &image_tex);
    glDeleteTextures(1, &color_tex);
    glDeleteFramebuffers(1, &fbo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imgui_context);
    glfwDestroyWindow(window2);

    return raw_image;
}

bool ButtonConditional(const char *label, bool cond = true, const ImVec2 &size = ImVec2(0, 0)) {
    if (cond) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
    }
    bool ret = ImGui::Button(label, size);
    if (cond) {
        ImGui::PopStyleColor();
    }
    return ret;
}

void PrintHelpAndExit(FILE *stream, int rc) {
    const char help_string[] =
        "ssedit - edit screenshots\n"
        "\n"
        "Usage:\n"
        "  ssedit [OPTIONS] [IN_FILE [OUT_FILE]]\n"
        "\n"
        "Options:\n"
        "  -f FORMAT     Specify output image format\n"
        "  -h            Display this message and exit\n"
        "  -V            Display version info and exit\n"
    ;

    fputs(help_string, stream);
    exit(rc);
}

void PrintVersionAndExit(int rc) {
    const char version_string[] =
        "ssedit:       " SSEDIT_VERSION              "\n"
        "imgui:        " "%s"                        "\n"
        "glfw:         " "%s"                        "\n"
        "libspng:      " SSEDIT_LIBSPNG_VERSION      "\n"
        "libturbojpeg: " SSEDIT_LIBTURBOJPEG_VERSION "\n"
        "libjxl:       " SSEDIT_LIBJXL_VERSION       "\n"
    ;

    printf(version_string, ImGui::GetVersion(), glfwGetVersionString());
    exit(rc);
}

int main(int argc, char **argv) {
    const char *input_filename = nullptr;
    int input_fd = -1;
    const char *output_filename = nullptr;
    int output_fd = -1;
    Format output_format = Format::PNG; // TODO: first enabled
    const char *config_path = nullptr;

    setlocale(LC_ALL, "");
    LogInit(INFO, stderr);

    int opt;
    while ((opt = getopt(argc, argv, ":f:c:Vh")) != -1) {
        switch (opt) {
        case 'f':
            output_format = FormatFromString(optarg);
            if (output_format == Format::INVALID) {
                LogPrint(ERR, "Invalid format: %s", optarg);
                return 1;
            }
            break;
        case 'c':
            config_path = optarg;
            break;
        case 'V':
            PrintVersionAndExit(0);
            break;
        case 'h':
            PrintHelpAndExit(stdout, 0);
            break;
        case '?':
            LogPrint(ERR, "Unknown option: %c", optopt);
            PrintHelpAndExit(stderr, 1);
            break;
        case ':':
            LogPrint(ERR, "Missing arg for %c", optopt);
            PrintHelpAndExit(stderr, 1);
            break;
        default:
            LogPrint(ERR, "Error while parsing command line options");
            PrintHelpAndExit(stderr, 1);
            break;
        }
    }

    if (argv[optind] != nullptr) {
        input_filename = argv[optind++];
    }
    if (argv[optind] != nullptr) {
        output_filename = argv[optind++];
    }

    if (input_filename == nullptr || STREQ(input_filename, "-")) {
        if (isatty(STDIN_FILENO)) {
            LogPrint(ERR, "Input file is not specified and stdin is a TTY");
            return 1;
        }
        input_fd = STDIN_FILENO;
    } else {
        input_fd = open(input_filename, O_RDONLY | O_CLOEXEC);
        if (input_fd < 0) {
            LogPrint(ERR, "Failed to open input file %s (%s)",
                     input_filename, strerror(errno));
            return 1;
        }
    }
    if (output_filename == nullptr || STREQ(output_filename, "-")) {
        if (isatty(STDOUT_FILENO)) {
            LogPrint(ERR, "Output file is not specified and stdout is a TTY");
            return 1;
        }
        output_fd = STDOUT_FILENO;
    } else {
        output_fd = open(output_filename,
                         O_WRONLY | O_TRUNC | O_CREAT | O_CLOEXEC,
                         S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP);
        if (output_fd < 0) {
            LogPrint(ERR, "Failed to open output file %s (%s)",
                     output_filename, strerror(errno));
            return 1;
        }
    }

    glfwSetErrorCallback(glfw_error_callback);
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
    if (glfwInit() != GLFW_TRUE) {
        LogPrint(ERR, "failed to initialize glfw!");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Create window with graphics context
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "ssedit");
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
        if (err != 4) {
            LogPrint(WARN, "Failed to init GLEW: %s (%d)", glewGetErrorString(err), err);
            return 1;
        }
        LogPrint(WARN, "GLEW init returned %d (%s)", err, glewGetErrorString(err));
        LogPrint(WARN, "This appears to be a bug: https://github.com/nigels-com/glew/issues/417");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext *imgui_context = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // disable automatic .ini file saving
    ImGuiStyle &style = ImGui::GetStyle();

    LoadConfig(config_path, &style);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load fonts
    ImVector<ImWchar> font_ranges;
    ImFontGlyphRangesBuilder font_ranges_builder;
    if (config.font_path != nullptr) {
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesGreek());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesThai());
        font_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
        font_ranges_builder.BuildRanges(&font_ranges);
        io.Fonts->AddFontFromFileTTF(config.font_path, config.font_size, nullptr, font_ranges.Data);
    } else {
        io.Fonts->AddFontDefault();
    }

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    font_config.MergeMode = true;
    font_config.GlyphOffset = ImVec2(0, 1);
    const ImWchar icon_ranges[] = { 0xe005, 0xf8ff, 0 };
    io.Fonts->AddFontFromMemoryTTF(icons_ttf_start, icons_ttf_size,
                                   19.f, &font_config, icon_ranges);

    size_t data_size;
    unsigned char *raw_data = ReadFromFD(input_fd, &data_size);
    if (raw_data == NULL) {
        return 1;
    }
    close(input_fd);

    Image *orig_image = DecodeImage(raw_data, data_size);
    if (orig_image == nullptr) {
        return 1;
    }
    free(raw_data);

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orig_image->w, orig_image->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, orig_image->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Main loop
    bool need_export = false;

    Tool active_tool = FREEFORM;
    float thickness = 2.0f;
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    bool fill = false;

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
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("ssedit", nullptr, ImGuiWindowFlags_NoMove
                                        | ImGuiWindowFlags_NoDecoration
                                        | ImGuiWindowFlags_NoSavedSettings);


        // Control pane on the left
        ImGui::BeginChild("Controls", ImVec2(300, 0), true);
        // Save it for later: https://github.com/ocornut/imgui/wiki/Tips#using-beginbeginchild
        ImGui::EndChild();

        // Image area
        ImGui::SameLine();
        ImGui::BeginChild("Canvas");

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetWindowSize();

        // Compute scale to fit image while preserving aspect ratio
        float image_scale = std::min(canvas_size.x / orig_image->w, canvas_size.y / orig_image->h);
        ImVec2 image_size = ImVec2(orig_image->w * image_scale, orig_image->h * image_scale);

        // Center the image inside the canvas region
        ImVec2 image_offset = (canvas_size - image_size) * 0.5f;
        ImVec2 image_pos = canvas_pos + image_offset;

        ImGui::SetCursorScreenPos(image_pos);
        ImGui::Image((uintptr_t)image_texture, image_size);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        for (const auto &shape: shapes) {
            shape->Draw(draw_list, image_pos, image_scale);
        }
        if (drawing_shape) {
            drawing_shape->Draw(draw_list, image_pos, image_scale);
        }

        if (ImGui::IsItemHovered()) {
            // Convert to image space
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            ImVec2 local_mouse_pos = (mouse_pos - image_pos) * (1.0f / image_scale);

            if (ImGui::IsMouseClicked(0) && !drawing_active) {
                drawing_active = true;
                drawing_start_pos = local_mouse_pos;

                switch (active_tool) {
                case LINE:
                    drawing_shape = std::make_unique<Line>(drawing_start_pos,
                                                           IMVEC4_TO_COL32(color),
                                                           thickness);
                    break;
                case CIRCLE:
                    drawing_shape = std::make_unique<Circle>(drawing_start_pos,
                                                             IMVEC4_TO_COL32(color),
                                                             thickness, fill);
                    break;
                case RECTANGLE:
                    drawing_shape = std::make_unique<Rectangle>(drawing_start_pos,
                                                                IMVEC4_TO_COL32(color),
                                                                thickness, fill);
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
                drawing_shape->Update(local_mouse_pos);

                if (ImGui::IsMouseReleased(0)) {
                    shapes.push_back(std::move(drawing_shape));
                    redo_list.clear();

                    drawing_shape.reset();
                    drawing_active = false;
                    drawing_start_pos = ImVec2(0, 0);
                }
            }
        }

        ImGui::EndChild();
        // End of image area

        // Return to the control pane context and draw it
        ImGui::BeginChild("Controls");

        ImGui::Text("Color");
        ImGui::SetNextItemWidth(-1);
        ImGui::ColorEdit4("##Color", (float *)&color);

        ImGui::Text("Thickness");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##Thickness", &thickness, 1.0f, 30.0f);

        ImGui::Text("Tool");
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_count = 5.0f;
        float spacing = style.ItemSpacing.x;
        float button_width = (available_width - (button_count - 1) * spacing) / button_count;
        if (ButtonConditional(ICON_PAINTBRUSH, active_tool == FREEFORM, ImVec2(button_width, 0))) {
            active_tool = FREEFORM;
        }
        ImGui::SameLine();
        if (ButtonConditional(ICON_SLASH, active_tool == LINE, ImVec2(button_width, 0))) {
            active_tool = LINE;
        }
        ImGui::SameLine();
        if (ButtonConditional(ICON_CIRCLE, active_tool == CIRCLE, ImVec2(button_width, 0))) {
            active_tool = CIRCLE;
        }
        ImGui::SameLine();
        if (ButtonConditional(ICON_SQUARE, active_tool == RECTANGLE, ImVec2(button_width, 0))) {
            active_tool = RECTANGLE;
        }
        ImGui::SameLine();
        if (ButtonConditional(ICON_ARROW_RIGHT, active_tool == ARROW, ImVec2(button_width, 0))) {
            active_tool = ARROW;
        }

        ImGui::Checkbox("Fill", &fill);

        available_width = ImGui::GetContentRegionAvail().x;
        button_count = 3.0f;
        spacing = ImGui::GetStyle().ItemSpacing.x;
        button_width = (available_width - (button_count - 1) * spacing) / button_count;
        ImGui::Text("Clear/Undo/Redo");
        if (ImGui::Button(ICON_XMARK, ImVec2(button_width, 0))) {
            while (Undo());
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_ROTATE_LEFT, ImVec2(button_width, 0))) {
            Undo();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_ROTATE_RIGHT, ImVec2(button_width, 0))) {
            Redo();
        }

        ImGui::Text("Copy to clibpoard");
        available_width = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button(ICON_COPY, ImVec2(available_width, 0))) {
            need_export = true;
        }

        ImGui::EndChild();
        // Leave control panel context

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (need_export) {
            need_export = false;

            Image *raw_image = GetModifiedPixels(orig_image);

            Image *encoded_image = EncodeImage(raw_image, output_format);
            delete raw_image;

            if (encoded_image != nullptr) {
                CopyToClipboard(encoded_image);
            }
            delete encoded_image;

            glfwMakeContextCurrent(window);
            ImGui::SetCurrentContext(imgui_context);
        }
    }

    Image *final_image = GetModifiedPixels(orig_image);
    delete orig_image;

    // Cleanup
    glfwMakeContextCurrent(window);
    ImGui::SetCurrentContext(imgui_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Delete OpenGL texture
    glDeleteTextures(1, &image_texture);

    // Destroy window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    Image *encoded_final_image = EncodeImage(final_image, output_format);
    delete final_image;

    if (encoded_final_image != nullptr) {
        WriteToFD(output_fd, encoded_final_image->data, encoded_final_image->data_size);
    }
    delete encoded_final_image;

    return 0;
}

