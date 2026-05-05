# Boomer (C Port)

A zoomer application for boomers, ported from Nim to C.

This version is a standalone C implementation that uses OpenGL and X11 to provide a smooth, hardware-accelerated zooming experience for your Linux desktop.

## Features

- **Smooth Zooming**: High-performance zooming anchored to your mouse cursor.
- **Flashlight Mode**: Highlight specific areas of the screen.
- **Modern Build System**: Built with Meson and Ninja for simplicity and speed.
- **No External Dependencies**: Shaders are embedded in the binary.

## Requirements

To build and run Boomer, you need the following development libraries:

- `X11`
- `Xrandr`
- `GL` (OpenGL)
- `pkg-config`
- `meson` and `ninja`

On Ubuntu/Debian, you can install these with:
```bash
sudo apt install libx11-dev libxrandr-dev libgl1-mesa-dev pkg-config meson ninja-build
```

## Building

1. **Configure the build directory**:
   ```bash
   meson setup build
   ```

2. **Compile the project**:
   ```bash
   meson compile -C build
   ```

The executable will be located at `./build/boomer`.

## Usage

```bash
Usage: boomer [OPTIONS]
  -d, --delay <seconds>     Delay execution by the provided seconds.
  -h, --help                Show help and exit.
  -c, --config <filepath>   Use config at <filepath>.
  -V, --version             Show the current version and exit.
  -w, --windowed            Run in windowed mode instead of fullscreen.
  --select                  Select a specific window to zoom into.
  --new-config [filepath]   Generate a new default config.
```

## Controls

### Keyboard
- `Esc` or `Q`: Quit.
- `z`: Toggle Flashlight mode.
- `0`: Reset zoom and position.
- `=`: Zoom In.
- `-`: Zoom Out.
- `Ctrl` + `=`: Increase Flashlight radius (if enabled).
- `Ctrl` + `-`: Decrease Flashlight radius (if enabled).

### Mouse
- **Hold Left Click & Drag**: Pan the view.
- **Scroll Up**: Zoom In.
- **Scroll Down**: Zoom Out.
- **Ctrl + Scroll Up**: Increase Flashlight radius (if enabled).
- **Ctrl + Scroll Down**: Decrease Flashlight radius (if enabled).

## Configuration

Boomer looks for a config file if specified via `-c`. You can generate a default one using:
```bash
./build/boomer --new-config my_config.conf
```

The config file supports:
- `min_scale`
- `scroll_speed`
- `drag_friction`
- `scale_friction`

## License

This project is licensed under the MIT License.
