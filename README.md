> [!WARNING]
> This project is kinda ass. Seriously, I really don't like how it turned out
> so I'm probably gonna rewrite it eventually. Don't recommend anyone to use it.
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
ImGui is licensed under the MIT license.
Font Awesome is licensed under the SIL OFL 1.1 license.
inih is licensed under the New BSD license.

## References
- https://github.com/jtheoof/swappy
- https://github.com/ocornut/imgui/tree/master/docs
- https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3
- https://learnopengl.com/
- https://libspng.org/docs/
- https://libjpeg-turbo.org/Documentation/Documentation
- https://github.com/libjxl/libjxl/tree/main/examples
- https://fontawesome.com
- https://fontforge.org/docs/scripting/scripting.html
- https://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967
- https://github.com/benhoyt/inih

[Dear ImGui]: https://github.com/ocornut/imgui
