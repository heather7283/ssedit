# ssedit
ssedit is a program for quick screenshot editing.
It uses [Dear ImGui] for user interface and glfw/opengl for rendering.

## Building
```
git clone https://github.com/heather7283/ssedit.git
cd ssedit
meson setup build
meson compile -C build
```

## Usage
```
grim - | ssedit | wl-copy
```

## License
ssedit is licensed under GNU GPL version 3 or later.
ImGui is licensed under MIT.

## References
- https://github.com/jtheoof/swappy
- https://github.com/ocornut/imgui/tree/master/docs
- https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3
- https://learnopengl.com/
- https://libspng.org/docs/
- https://libjpeg-turbo.org/Documentation/Documentation
- https://github.com/libjxl/libjxl/tree/main/examples

[Dear ImGui]: https://github.com/ocornut/imgui
