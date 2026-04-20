# Ray Tracer
A high-performance, multithreaded C++ Ray Tracer that renders 3D meshes defined in XML scene files. It supports smooth shading, texture mapping, and various light sources.

## 📂 Project Structure
```
RayTracer/
├── bin/                # Executable binaries
├── build/              # Compiled object files (.o)
├── data/               # Scene XML files and textures (e.g., clay.jpg)
├── external/           # Third-party libraries (TinyXML2, STB)
├── include/            # Header files (.h)
├── src/                # Source code (.cpp)
└── Makefile            # Build configuration
```

## 🛠 Prerequisites
Ensure you have the following installed:
- **GCC/G++ Compiler** (C++11 or higher supported)
- **Make** (Use `mingw32-make` if on Windows)
- **Pthreads library** (Standard on Linux; included in MinGW for Windows)

## 🚀 Compilation
The project includes an optimized Makefile that uses the `-O3` flag for maximum rendering speed.

1. Open a terminal in the project root directory.
2. Build the project:
```bash
make
```
3. (Optional) To clean the build files and start fresh:
```bash
make clean
```

## 🖥 Usage
The program can be run with a default scene or a custom XML file passed as a command-line argument.

### Default Execution
By default, the program looks for `data/scene.xml`:
```bash
./bin/raytracer
```

### Custom Scene
To run a specific scene file:
```bash
./bin/raytracer data/your_scene_file.xml
```

## 📊 Sample Output
When successful, the terminal will display the scene statistics and rendering progress:
```
Successfully loaded: data/scene.xml

--- Scene Data Check ---
Max Depth: 6
Camera Res: 1200x1200
Vertices loaded: 621
Materials loaded: 2
Lights loaded: 3
Meshes loaded: 3
...
Texture loaded: data/clay.jpg
Rendering 1200x1200 image with 4 threads...
All threads finished.
Render time: 7.145 seconds.
Success! Saved to output.png
```

## ✨ Features & Technical Details
- **Multithreading:** Uses `pthread` to utilize all CPU cores, significantly reducing render times.
- **Intersection Algorithm:** Implements the Möller–Trumbore ray-triangle intersection for high efficiency.
- **Shading Models:**
  - **Blinn-Phong:** Realistic specular highlights and diffuse reflections.
  - **Smooth Shading:** Normal interpolation across triangle faces for curved surfaces.
- **Light Sources:** Supports Point Lights (with $1/d^2$ attenuation), Triangular (Planar) Lights, and Ambient Lighting.
- **Shadows:** Accurate shadow casting with geometric offsets to prevent "Shadow Acne" artifacts.
- **Texture Mapping:** Bilinear UV interpolation with configurable `texturefactor` for blending.
- **Image Output:** Saves high-resolution results in `.png` format via the `stb_image_write` library.

---
*Developed for CSE 461 Computer Graphics.*