# Magnesium

Magnesium is a modular, Component Entity System based game engine you can use with any libraries you build wrappers for. It comes with support for [PlaneShader](https://github.com/Black-Sphere-Studios/planeshader), [Box2D](https://github.com/Black-Sphere-Studios/Box2D), [LuaJIT](http://luajit.org/), and [TinyOAL](https://github.com/Black-Sphere-Studios/tinyoal).

## Compiling

Because Magnesium has so many dependencies, a [vcpkg port](https://github.com/Black-Sphere-Studios/vcpkg/tree/master/ports/magnesium) is provided to automatically acquire and build all necessary components. Simply clone the [Black Sphere Studios vcpkg fork](https://github.com/Black-Sphere-Studios/vcpkg) (or merge its changes into your own vcpkg fork), run `.\bootstrap-vcpkg.bat` and then `.\vcpkg.exe install magnesium:x64-windows` (or `magnesium:x86-windows` for the 32-bit versions). Magnesium will then be ready to be used by any `vcpkg` enabled Visual Studio game project. For more information on vcpkg, [check it's readme](https://github.com/Black-Sphere-Studios/vcpkg/blob/master/README.md).

If you would like to build and test the examples, you will need to [download the SDK](https://github.com/Black-Sphere-Studios/magnesium/releases) that has the example media files. You can either install the vcpkg version of magnesium and let the Visual Studio project override it, or you can install only magnesium's dependencies: `.\vcpkg.exe install bss-util:x64-windows, planeshader:x64-windows, tinyoal:x64-windows, box2d-bss:x64-windows, luajit:x64-windows, enet:x64-windows`. Once these dependencies are installed, the SDK project file will automatically link to them, provided you've enabled [vcpkg Visual Studio integration](https://github.com/Microsoft/vcpkg/blob/master/README.md#quick-start).

©2018 Erik McClure