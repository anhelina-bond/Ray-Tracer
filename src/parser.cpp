#include "tinyxml2.h"
#include "scene.h"
#include <sstream>

using namespace tinyxml2;

void loadScene(const char* filename, Scene& scene) {
    XMLDocument doc;
    XMLError eResult = doc.LoadFile(filename);
    
    if (eResult != XML_SUCCESS) {
        std::cerr << "Error: Could not load XML file: " << filename << std::endl;
        std::cerr << "Error ID: " << eResult << std::endl;
        return; // This is why nx and ny remain 0
    }
    std::cout << "Successfully loaded: " << filename << std::endl;

    XMLElement* root = doc.FirstChildElement("scene");
    
    // 1. Get maxraytracedepth
    XMLElement* depthElement = root->FirstChildElement("maxraytracedepth");
    if (depthElement) {
        scene.max_ray_trace_depth = depthElement->IntText();
    }

    // 2. Get Background Color
    XMLElement* bgElement = root->FirstChildElement("backgroundColor");
    if (bgElement) {
        std::stringstream ss(bgElement->GetText());
        ss >> scene.background_color.e[0] >> scene.background_color.e[1] >> scene.background_color.e[2];
    }

    XMLElement* camElement = root->FirstChildElement("camera");
    if (camElement) {
        // 1. Declare variables outside the IF blocks so they exist for the final call
        point3 position;
        vec3 gaze, up;
        double left, right, bottom, top;
        double near_distance;
        int nx, ny;

        // Helper for x,y,z parsing
        double x, y, z;

        // 2. Parse Position
        XMLElement* positionElement = camElement->FirstChildElement("position");
        if (positionElement && positionElement->GetText()) {
            std::stringstream ss(positionElement->GetText());
            ss >> x >> y >> z;
            position = point3(x, y, z); // Assignment, not function call
        }

        // 3. Parse Gaze
        XMLElement* gazeElement = camElement->FirstChildElement("gaze");
        if (gazeElement && gazeElement->GetText()) {
            std::stringstream ss(gazeElement->GetText());
            ss >> x >> y >> z;
            gaze = vec3(x, y, z);
        }

        // 4. Parse Up
        XMLElement* upElement = camElement->FirstChildElement("up");
        if (upElement && upElement->GetText()) {
            std::stringstream ss(upElement->GetText());
            ss >> x >> y >> z;
            up = vec3(x, y, z);
        }

        // 5. Parse Nearplane (4 values)
        XMLElement* nearplaneElement = camElement->FirstChildElement("nearPlane");
        if (nearplaneElement && nearplaneElement->GetText()) {
            std::stringstream ss(nearplaneElement->GetText());
            ss >> left >> right >> bottom >> top;
        }

        // 6. Parse Near Distance (1 value)
        XMLElement* neardistanceElement = camElement->FirstChildElement("neardistance");
        if (neardistanceElement && neardistanceElement->GetText()) {
            std::stringstream ss(neardistanceElement->GetText());
            ss >> near_distance;
        }

        // 7. Parse Image Resolution (2 values)
        XMLElement* imageresolutionElement = camElement->FirstChildElement("imageresolution");
        if (imageresolutionElement && imageresolutionElement->GetText()) {
            std::stringstream ss(imageresolutionElement->GetText());
            ss >> nx >> ny;
        }

        // 8. Update the Scene's Camera
        // Assuming 'cam' is a public member of Scene
        scene.cam = Camera(position, gaze, up, left, right, bottom, top, near_distance, nx, ny);
    }
    XMLElement* lightsElement = root->FirstChildElement("lights");
    if (lightsElement) {

        // --- 1. Handle Ambient Light ---
        XMLElement* ambElem = lightsElement->FirstChildElement("ambientlight");
        if (ambElem && ambElem->GetText()) {
            double r, g, b;
            std::stringstream ss(ambElem->GetText());
            ss >> r >> g >> b;
            // Usually, ambient light is stored as a single global color in the scene
            scene.ambient_light = color(r, g, b); 
        }

        // --- 2. Handle Point Lights ---
        for (XMLElement* pElem = lightsElement->FirstChildElement("pointlight"); 
            pElem != nullptr; 
            pElem = pElem->NextSiblingElement("pointlight")) {
            
            double px, py, pz, ir, ig, ib;

            XMLElement* posElem = pElem->FirstChildElement("position");
            XMLElement* intElem = pElem->FirstChildElement("intensity");

            if (posElem && intElem) {
                std::stringstream ssPos(posElem->GetText());
                ssPos >> px >> py >> pz;
                
                std::stringstream ssInt(intElem->GetText());
                ssInt >> ir >> ig >> ib;

                // Create pointLight and push to polymorphic vector
                scene.lights.push_back(std::make_shared<pointLight>(ir, ig, ib, px, py, pz));
            }
        }

        // --- 3. Handle Triangular Lights ---
        for (XMLElement* tElem = lightsElement->FirstChildElement("triangularlight"); 
            tElem != nullptr; 
            tElem = tElem->NextSiblingElement("triangularlight")) {
            
            point3 v1, v2, v3;
            double ir, ig, ib;

            // Parse 3 vertices and intensity
            XMLElement* v1Elem = tElem->FirstChildElement("vertex1");
            XMLElement* v2Elem = tElem->FirstChildElement("vertex2");
            XMLElement* v3Elem = tElem->FirstChildElement("vertex3");
            XMLElement* intElem = tElem->FirstChildElement("intensity");

            if (v1Elem && v2Elem && v3Elem && intElem) {
                std::stringstream ss1(v1Elem->GetText()); ss1 >> v1.e[0] >> v1.e[1] >> v1.e[2];
                std::stringstream ss2(v2Elem->GetText()); ss2 >> v2.e[0] >> v2.e[1] >> v2.e[2];
                std::stringstream ss3(v3Elem->GetText()); ss3 >> v3.e[0] >> v3.e[1] >> v3.e[2];
                std::stringstream ssI(intElem->GetText()); ssI >> ir >> ig >> ib;

                // Create triangularLight and push to polymorphic vector
                scene.lights.push_back(std::make_shared<triangularLight>(ir, ig, ib, v1, v2, v3));
            }
        }
    }

    XMLElement* materialElement = root->FirstChildElement("materials");
    if (materialElement) {
        for (XMLElement* pElem = materialElement->FirstChildElement("material"); 
             pElem != nullptr; 
             pElem = pElem->NextSiblingElement("material")) {
            
            color amb, diff, spec, mirr;
            double phong = 0, texFact = 0;

            XMLElement* ambE  = pElem->FirstChildElement("ambient");
            XMLElement* diffE = pElem->FirstChildElement("diffuse");
            XMLElement* specE = pElem->FirstChildElement("specular");
            XMLElement* mirrE = pElem->FirstChildElement("mirrorreflectance");
            XMLElement* phonE = pElem->FirstChildElement("phongexponent");
            XMLElement* texFE = pElem->FirstChildElement("texturefactor");

            // Check if every single tag exists
            if (ambE && diffE && specE && mirrE && phonE && texFE) {
                std::stringstream ssA(ambE->GetText()); ssA >> amb.e[0] >> amb.e[1] >> amb.e[2];
                std::stringstream ssD(diffE->GetText()); ssD >> diff.e[0] >> diff.e[1] >> diff.e[2];
                std::stringstream ssS(specE->GetText()); ssS >> spec.e[0] >> spec.e[1] >> spec.e[2];
                std::stringstream ssM(mirrE->GetText()); ssM >> mirr.e[0] >> mirr.e[1] >> mirr.e[2];
                
                phong = phonE->DoubleText();
                texFact = texFE->DoubleText();

                scene.add_material(Material(amb, diff, spec, phong, mirr, texFact));
            } else {
                std::cerr << "Warning: A material tag was missing sub-elements!" << std::endl;
            }
        }
    }

    XMLElement* vertexElement = root->FirstChildElement("vertexdata");
    if (vertexElement) {
        std::stringstream ss(vertexElement->GetText());
        double x, y, z;
        while (ss >> x >> y >> z) {
            scene.add_vertex(vertex(x, y, z));
        }
    }

    XMLElement* textureElement = root->FirstChildElement("texturedata");
    if (textureElement) {
        std::stringstream ss(textureElement->GetText());
        double u, v;
        while (ss >> u >> v) {
            scene.add_texture(Texture(u, v));
        }
    }

    XMLElement* texImgElem = root->FirstChildElement("textureimage");
        if (texImgElem && texImgElem->GetText()) {
            std::stringstream ss(texImgElem->GetText());
            ss >> scene.texture_image_name;
        }

    XMLElement* normalElement = root->FirstChildElement("normaldata");
    if (normalElement) {
        std::stringstream ss(normalElement->GetText());
        double x, y, z;
        while (ss >> x >> y >> z) {
            scene.add_normal(vec3(x, y, z));
        }
    }

    XMLElement* objectsElem = root->FirstChildElement("objects");
    if (objectsElem) {
        // Loop through every <mesh> tag inside <objects>
        for (XMLElement* meshElem = objectsElem->FirstChildElement("mesh"); 
            meshElem != nullptr; 
            meshElem = meshElem->NextSiblingElement("mesh")) {

            // 1. Get Material ID
            int matId = 0;
            XMLElement* matIdElem = meshElem->FirstChildElement("materialid");
            if (matIdElem) {
                matId = matIdElem->IntText();
            }

            // Create a mesh instance with this material ID
            Mesh currentMesh(matId);

            // 2. Parse Faces
            XMLElement* facesElem = meshElem->FirstChildElement("faces");
            if (facesElem && facesElem->GetText()) {
                std::stringstream ss(facesElem->GetText());
                std::string v1_block, v2_block, v3_block;

                // In XML, a face is defined by 3 vertices (v/t/n)
                while (ss >> v1_block >> v2_block >> v3_block) {
                    int v[3], t[3], n[3];

                    sscanf(v1_block.c_str(), "%d/%d/%d", &v[0], &t[0], &n[0]);
                    sscanf(v2_block.c_str(), "%d/%d/%d", &v[1], &t[1], &n[1]);
                    sscanf(v3_block.c_str(), "%d/%d/%d", &v[2], &t[2], &n[2]);

                    // Add the face to the current mesh
                    currentMesh.add_face(v[0], v[1], v[2], 
                                        t[0], t[1], t[2], 
                                        n[0], n[1], n[2]);
                }
            }

            // Add the completed mesh to the scene
            scene.add_mesh(currentMesh);
        }
    }


    // --- DEBUG PRINTS ---
    std::cout << "\n--- Scene Data Check ---" << std::endl;
    std::cout << "Max Depth: " << scene.max_ray_trace_depth << std::endl;
    std::cout << "Camera Res: " << scene.cam.nx << "x" << scene.cam.ny << std::endl;
    std::cout << "Vertices loaded: " << scene.vertex_data.size() << std::endl;
    std::cout << "Materials loaded: " << scene.materials.size() << std::endl;
    std::cout << "Lights loaded: " << scene.lights.size() << std::endl;
    std::cout << "Meshes loaded: " << scene.meshes.size() << std::endl;
    if (!scene.meshes.empty()) {
        for (int i = 0; i < scene.meshes.size(); i++) {
            std::cout << "Faces in Mesh " << i <<  ": " << scene.meshes[i].faces.size() << std::endl;
        }
        
    }
    std::cout << "------------------------\n" << std::endl;
}