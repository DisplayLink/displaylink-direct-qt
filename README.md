# DisplayLink Direct - Qt Platform Plugin
Qt 6 platform plugin for rendering Qt applications through DisplayLink Direct.

The plugin exposes one persistent Qt screen. The first usable DLSDK display is
attached to it, while additional displays are retained as failover candidates.

## Requirements
- CMake 3.15 or newer
- A C++20 compiler
- DisplayLink Direct 2.0.0 or newer
- Qt 6 with development packages

## Building
Configure the project:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="<path/to/displaylink-direct>"
```

Build it:
```bash
cmake --build build --parallel
```

Available build options:
- `BUILD_PLUGIN` - builds the Qt platform plugin (default `ON`)
- `BUILD_TESTS` - builds unit tests (default `ON`)
- `QDLSDK_PLUGIN_INSTALL_DIR` - overrides the default plugin installation directory. Must end in `platforms`.

For example, pass `-DBUILD_TESTS=OFF` when only the plugin binary is needed.

## Testing
Build and run the configured tests with:
```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Usage
The build places the plugin at:
```
build/src/plugin/plugins/platforms/libqdlsdk.so
```

Set `QT_PLUGIN_PATH` to the parent directory containing `platforms`, then select the plugin with `-platform dlsdk`:
```bash
QT_PLUGIN_PATH="$PWD/build/src/plugin/plugins" ./path/to/qt-app -platform dlsdk
```

Alternatively, specify `QT_QPA_PLATFORM` like so:
```bash
QT_PLUGIN_PATH="$PWD/build/src/plugin/plugins" QT_QPA_PLATFORM=dlsdk ./path/to/qt-app
```

If the plugin is installed in a standard system location (see installation information below), setting `QT_PLUGIN_PATH`
is no longer necessary.

> [!TIP]
> The bundled dev container installs the `qt6-base-examples` package, which contains some Qt sample apps useful for 
> trying out the platform plugin. They are located in `/usr/lib/x86_64-linux-gnu/qt6/examples`.

## Installing
By default, CMake installs the plugin directly into the `platforms` directory of the Qt installation selected during
configuration. This is the recommended approach because applications using that Qt instance can discover the plugin
without setting `QT_PLUGIN_PATH`:
```bash
cmake --build build --parallel
cmake --install build
```

The Qt plugin directory may require elevated write permission.

Set `QDLSDK_PLUGIN_INSTALL_DIR` to change the plugin installation destination. For example, to install the plugin for
the current user:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_PREFIX_PATH="<path-to-dlsdk>" \
	-DQDLSDK_PLUGIN_INSTALL_DIR="$HOME/.local/lib/qt6/plugins/platforms"
cmake --build build --parallel
cmake --install build
```

When using a custom destination, remember to point `QT_PLUGIN_PATH` at its parent plugin directory.

**Note:** Installing the Qt plugin does not install DisplayLink Direct. The DisplayLink Direct runtime library must
be available to the installed plugin through standard system paths or `LD_LIBRARY_PATH`.

## Platform Arguments
Platform arguments follow the Qt `platform:argument` syntax. Supported arguments are:
- `cursor` - enables rendering of Qt's software cursor. Disabled by default.
- `dpi=<value>` - sets the logical horizontal and vertical DPI. Default value is 96.

Arguments can be combined by separating them with colons:
```bash
./path/to/qt-app -platform dlsdk:cursor:dpi=144
```

They can also be supplied through the environment variable:
```bash
QT_QPA_PLATFORM=dlsdk:cursor:dpi=144 ./path/to/qt-app
```

## Developing
The recommended way to work on the plugin is to use the bundled dev container. The container includes the compiler,
build systems, Qt dev packages, debugging utilities and VS Code extensions. It's possible to run Qt apps directly from
the container.

DisplayLink Direct is not installed in the container. Make it available in the mounted workspace and pass its location
through `CMAKE_PREFIX_PATH`, then use the normal build and test commands described above.

**Note:** If the `displaylink` group owns the device files in `/dev/bus/usb/*` on the host system, Qt applications
may need to run with `sudo` inside the container. Alternatively, a group with a matching GID can be created inside the
container and the `vscode` user added to it.

## Logging
Set `QT_DEBUG_PLUGINS=1` to diagnose Qt plugin discovery and loading:
```bash
QT_DEBUG_PLUGINS=1 ./path/to/qt-app -platform dlsdk
```

The plugin logs messages using these Qt logging categories:
- `qdlsdk.plugin`
- `qdlsdk.display`
- `qdlsdk.input`
- `qdlsdk.draw`

Control them with `QT_LOGGING_RULES`. For example, to disable render debug messages:
```bash
QT_LOGGING_RULES="qdlsdk.draw.debug=false" ./path/to/qt-app -platform dlsdk
```

Multiple rules are separated with semicolons.
