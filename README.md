<br/>
<p align="center">
  <img src="https://user-images.githubusercontent.com/3519379/173830836-d390e164-fe59-4b9d-950a-c28fa3ebc037.png">
</p>
<br/>

<div align="center">

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT) 
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/c0c10a437a214dbf963210ed3edf3c4f)](https://www.codacy.com/gh/inanevin/LinaEngine/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=inanevin/LinaEngine&amp;utm_campaign=Badge_Grade)
[![Build Status](https://app.travis-ci.com/inanevin/LinaEngine.svg?branch=master)](https://app.travis-ci.com/inanevin/LinaEngine)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/inanevin/LinaEngine/issues) 
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/inanevin) 
[![Lina Discord](https://badgen.net/discord/members/QYeTkEtRMB)](https://discord.gg/QYeTkEtRMB)
![GitHub issues](https://img.shields.io/github/issues/inanevin/LinaEngine.svg)
![GitHub repo size](https://img.shields.io/github/repo-size/inanevin/LinaEngine.svg)

 </div>
  
Lina Engine is a lightweight and ECS based open-source game engine. Initially it's a research project and is aimed to be a basis of development for more open-source game engine projects. Lina's architecture is designed to be as modular as possible and it focuses on creating replacable modules to customize for project-specific needs, rather than stacking thousands of features into one framework.

[Visit the project page on my website for more information](https://www.inanevin.com/gameprojects/linaengine)

![Lina](Docs/Images/lina_ss.png)
![Lina](Docs/Images/linass1.png)

## Announcements / Notes

- Lina Engine is currently compatible to be compiled only with x64 architecture on Windows. If you would like to use it on different architectures & platforms, you need to compile the vendors for your target architecture and platform include them manually.

- Lina is currently in active development so it's not stable on all platforms. It's being developed in Visual Studio 2019 in Windows 10 with x64 architecture, so that's the best setup to contribute to the project for now.

## Lina Engine Tools

- [Lina Engine Build Launcher](https://github.com/inanevin/Lina-Engine-Build-Launcher) (deprecated)

## External Dependencies

-  [assimp](https://github.com/assimp/assimp)
-  [alut](http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0/doc/alut.html)
-  [fmt](https://github.com/fmtlib/fmt)
-  [glm](https://github.com/g-truc/glm)
-  [Nvidia PhysX](https://developer.nvidia.com/physx-sdk)
-  [openal](https://www.openal.org)
-  [stb](https://github.com/nothings/stb)
-  [taskflow](https://github.com/taskflow/taskflow)

## Installation

-  Clone Lina Engine git repository

```shell
# Initialize git in your desired directory.
git init

# Clone Lina Engine
git clone https://github.com/inanevin/LinaEngine

# Alternatively you can clone recursively, this will download all the submodules if there exists any along with Lina Engine source code.
git clone --recursive https://github.com/inanevin/LinaEngine

```
-  You can generate the project files and build using; ~~**Lina Engine Build Launcher (deprecated)**~~, **CMake with Shell** or **CMake GUI**

#### Using Lina Engine Build Launcher (deprecated)

-  You need to have JavaFX Runtime library installed on your computer & environment paths set for it. For more information visit [Java FX](https://openjfx.io/).
-  Download a binary release from [Lina Engine Build Launcher Repository](https://github.com/inanevin/Lina-Engine-Build-Launcher).
-  Run the .jar file, launcher will open.

![](Docs/Images/buildlauncher.png)

-  Select the directory where you downloaded Lina Engine's source. (this repository)
-  Select your desired build directory for project files.
-  Select your target generator & desired build options.
-  Hit "Generate Project Files" to generate only the project files, hit "Generate and Build" to generate project files and then build the binaries using the generated files.


#### Using CMake with Shell

-  Run your shell, command line or terminal in the repository directory. Afterwards, execute generation / build commands.

```shell
# You can define any options before generating project files.
cmake -DLINA_ENABLE_LOGGING=OFF

# You can define multiple options sequentially.
cmake -DLINA_ENABLE_LOGGING=OFF -DLINA_ENABLE_PROFILING=OFF

# It is recommended to declare the type of the option if multiple options are defined
cmake -DLINA_ENABLE_LOGGING:BOOL=OFF -DLINA_ENABLE_PROFILING:BOOL=OFF -DCMAKE_CONFIGURATION_TYPES:STRING="Debug,Release"

# Above commands will generate project files with default generator, you can specify a generator if you want.
cmake -DLINA_ENABLE_LOGGING=OFF -G "Visual Studio 15 2017"

```
-  After generating project files you can either open your IDE and build the ALL_BUILD project which will build all the targets or you can build the binaries from shell.

```shell
# Create a directory for binaries
mkdir bin

# Navigate to directory
cd bin

# Build the root project
cmake ../ -G "Visual Studio 16 2019" -A "x64"

# After the project files are built, you can build the project via
cmake --build . --target ALL_BUILD

```
-  Check out [CMake Documentation](https://cmake.org/cmake/help/v3.2/manual/cmake.1.html) for specifying more options and configurations like project file generation directories and more.

#### Using CMake GUI

-  Choose the directory where the repository is as the source directory.
-  Choose a build directory, this can be the same as the source directory or anywhere you like.
-  Hit configure, select the IDE you want to generate the project files for and x64 architecture.
-  Select options, you can leave them as default.
-  Hit generate, this will generate the project files for your desired IDE.

## Build Options

| Option  | Description | Default |
| ------------- | ------------- | ------------- |
| LINA_ENABLE_EDITOR | Launches the sandbox in editor mode.  | ON  |
| LINA_ENABLE_LOGGING | Enables log features for core modules.  | ON |
| LINA_ENABLE_PROFILING  | Enables profiler integration & profile data serialization. | ON |
| LINA_PRODUCTION_BUILD | Disable debug error checking & reporting. | OFF |
| CMAKE_CONFIGURATION_TYPES | Config types that will be available on the IDE. | Debug, Release, MinSizeRel, RelWithDebInfo  

## License

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT) 
Check out [Licence](LICENSE) file.

## Contributing

Please check out [Contributing](CONTRIBUTING.md) for further information.

## Design Diagrams

### System Architecture
![Diagram 1](Docs/Images/SCS.png?raw=true "Title")

### Design Document

[Link to the Software Design Document of Lina Engine (PDF)](https://www.inanevin.com/docs/lina_sdd.pdf)

[Lina Engine Project Report (PDF)](https://www.inanevin.com/docs/lina_postmortem.pdf)

