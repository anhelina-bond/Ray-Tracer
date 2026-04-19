#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <pthread.h>
#include <limits>
#include <fstream>

#include "scene.h"
#include "ray.h"
#include "parser.h"
#include "mesh.h"

#include <chrono>   
#include <iomanip>  
#include <stdlib.h>

// Structure to pass data to each thread
struct ThreadData {
    int start_row;
    int end_row;
    const Scene* scene;
    unsigned char* image_buffer;
};

// Helper: Clamps a double to [0, 255] for image output
unsigned char clamp(double val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (unsigned char)val;
}


// THE THREAD WORKER FUNCTION
void* render_rows(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int nx = data->scene->cam.nx;
    // int ny = data->scene->cam.ny;

    for (int j = data->start_row; j < data->end_row; ++j) {
        for (int i = 0; i < nx; ++i) {
            // Invert J because image coords usually start from top
            Ray r = data->scene->cam.get_ray(i, j);
            color pixel_color = trace(r, *(data->scene), 0);

            // STB expects [R, G, B, R, G, B...]
            int pixel_index = (j * nx + i) * 3;
            data->image_buffer[pixel_index + 0] = clamp(pixel_color.e[0]);
            data->image_buffer[pixel_index + 1] = clamp(pixel_color.e[1]);
            data->image_buffer[pixel_index + 2] = clamp(pixel_color.e[2]);
        }
    }
    return nullptr;
}


int main(int argc, char** argv) {
    char* scene_path = new char[50];
    if (argc == 0) {
         strcpy(scene_path, "data/scene.xml");
    } else{
        strcpy(scene_path, argv[1]);

    }
    
    Scene scene;
    loadScene(scene_path, scene);

    delete[] scene_path;
    int nx = scene.cam.nx;
    int ny = scene.cam.ny;

    if (nx <= 0 || ny <= 0) {
        std::cerr << "Error: Invalid image resolution." << std::endl;
        return 1;
    }

    unsigned char* image_buffer = new unsigned char[nx * ny * 3];

    // MULTITHREADING SETUP
    const int num_threads = 8; 
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int rows_per_thread = ny / num_threads;

    int width, height, channels;
    unsigned char* data = stbi_load(scene.texture_image_name.c_str(), &width, &height, &channels, 3);

    if (data) {
        scene.texture_data_buffer = data;
        scene.tex_width = width;
        scene.tex_height = height;
        std::cout << "Texture loaded: " << scene.texture_image_name << std::endl;
    }

    std::cout << "Rendering " << nx << "x" << ny << " image with " << num_threads << " threads..." << std::endl;

    // --- START TIMER ---
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].scene = &scene;
        thread_data[i].image_buffer = image_buffer;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? ny : (i + 1) * rows_per_thread;

        pthread_create(&threads[i], nullptr, render_rows, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // --- STOP TIMER ---
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "All threads finished." << std::endl;
    std::cout << "Render time: " << std::fixed << std::setprecision(3) << elapsed.count() << " seconds." << std::endl;

    // Save image
    int result = stbi_write_png("output.png", nx, ny, 3, image_buffer, nx * 3);
    
    if (result) std::cout << "Success! Saved to output.png" << std::endl;
    else std::cout << "Failed to write PNG!" << std::endl;

    delete[] image_buffer;
    if (data) stbi_image_free(data); // Good practice to free loaded texture

    return 0;
}