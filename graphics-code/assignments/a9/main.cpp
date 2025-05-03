#include "Common.h"
#include "OpenGLCommon.h"
#include "OpenGLMarkerObjects.h"
#include "OpenGLBgEffect.h"
#include "OpenGLMesh.h"
#include "OpenGLViewer.h"
#include "OpenGLWindow.h"
#include "TinyObjLoader.h"
#include "OpenGLSkybox.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>
#include <string>

#ifndef __Main_cpp__
#define __Main_cpp__

#ifdef __APPLE__
#define CLOCKS_PER_SEC 100000
#endif

#define M_PI 3.14159265358979323846264338327950288

class MyDriver : public OpenGLViewer
{
    std::vector<OpenGLTriangleMesh *> mesh_object_array;
    OpenGLBgEffect *bgEffect = nullptr;
    OpenGLSkybox *skybox = nullptr;
    clock_t startTime;

public:
    virtual void Initialize()
    {
        draw_axes = false;
        startTime = clock();
        OpenGLViewer::Initialize();
    }

    virtual void Initialize_Data()
    {
        //// Load all the shaders you need for the scene 
        //// In the function call of Add_Shader_From_File(), we specify three names: 
        //// (1) vertex shader file name
        //// (2) fragment shader file name
        //// (3) shader name used in the shader library
        //// When we bind a shader to an object, we implement it as follows:
        //// object->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("shader_name"));
        //// Here "shader_name" needs to be one of the shader names you created previously with Add_Shader_From_File()

        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/basic.frag", "basic");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/environment.frag", "environment");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/stars.vert", "shaders/stars.frag", "stars");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/alphablend.frag", "blend");

        //// Load all the textures you need for the scene
        //// In the function call of Add_Shader_From_File(), we specify two names:
        //// (1) the texture's file name
        //// (2) the texture used in the texture library
        //// When we bind a texture to an object, we implement it as follows:
        //// object->Add_Texture("tex_sampler", OpenGLTextureLibrary::Get_Texture("tex_name"));
        //// Here "tex_sampler" is the name of the texture sampler2D you used in your shader, and
        //// "tex_name" needs to be one of the texture names you created previously with Add_Texture_From_File()

        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_color.png", "earth_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/5.jpg", "sphere_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/jupiter.jpg", "jupiter_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/venus.jpg", "venus_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/mars.jpg", "mars_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/mercury.jpg", "mercury_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/sun.jpg", "sun_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/uranus.jpg", "uranus_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/neptune.jpg", "neptune_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon.jpg", "moon_color");

        //// Add all the lights you need for the scene (no more than 4 lights)
        //// The four parameters are position, ambient, diffuse, and specular.
        //// The lights you declared here will be synchronized to all shaders in uniform lights.
        //// You may access these lights using lt[0].pos, lt[1].amb, lt[1].dif, etc.
        //// You can also create your own lights by directly declaring them in a shader without using Add_Light().
        //// Here we declared three default lights for you. Feel free to add/delete/change them at your will.

        opengl_window->Add_Light(Vector3f(3, 1, 3), Vector3f(0.1, 0.1, 0.1), Vector3f(1, 1, 1), Vector3f(0.5, 0.5, 0.5)); 
        opengl_window->Add_Light(Vector3f(0, 0, -5), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));
        opengl_window->Add_Light(Vector3f(-5, 1, 3), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));

        //// Add the background / environment
        //// Here we provide you with four default options to create the background of your scene:
        //// (1) Gradient color (like A1 and A2; if you want a simple background, use this one)
        //// (2) Programmable Canvas (like A7 and A8; if you consider implementing noise or particles for the background, use this one)
        //// (3) Sky box (cubemap; if you want to load six background images for a skybox, use this one)
        //// (4) Sky sphere (if you want to implement a sky sphere, enlarge the size of the sphere to make it colver the entire scene and update its shaders for texture colors)
        //// By default, Option (2) (Buzz stars) is turned on, and all the other three are commented out.
        
        //// Background Option (1): Gradient color
        /*
        {
            auto bg = Add_Interactive_Object<OpenGLBackground>();
            bg->Set_Color(OpenGLColor(0.1f, 0.1f, 0.1f, 1.f), OpenGLColor(0.3f, 0.1f, .1f, 1.f));
            bg->Initialize();
        }
        */

        //// Background Option (2): Programmable Canvas
        //// By default, we load a GT buzz + a number of stars
        
        {
            bgEffect = Add_Interactive_Object<OpenGLBgEffect>();
            bgEffect->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("stars"));
            bgEffect->Initialize();
        }
        
        
        //// Background Option (3): Sky box
        //// Here we provide a default implementation of a sky box; customize it for your own sky box
        /*
        {
            // from https://www.humus.name/index.php?page=Textures
            const std::vector<std::string> cubemap_files{
                "cubemap/posx.jpg",     //// + X
                "cubemap/negx.jpg",     //// - X
                "cubemap/posy.jpg",     //// + Y
                "cubemap/negy.jpg",     //// - Y
                "cubemap/posz.jpg",     //// + Z
                "cubemap/negz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
        }
        */

        //// Background Option (4): Sky sphere
        //// Here we provide a default implementation of a textured sphere; customize it for your own sky sphere

        // Scale factors relative to Earth's diameter (12,742 km)
        float earthScale = 0.6f;  // Base scale
        float mercuryScale = 0.4f;
        float venusScale = 0.5f;
        float marsScale = 0.45f;
        float jupiterScale = 1.1f;
        float uranusScale = 0.65f;
        float neptuneScale = 0.65f;
        float sunScale = 0.8f;  // Multiplied by 5 to make it more visible
        float moonScale = 0.35f;

        //Position set-up
        float p_num = 8;
        float radius = 3.f;
        float angle = 0;

        //EARTH
        {
            //// create object by reading an obj mesh
            auto earth = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << earthScale, 0, 0, radius * cos(angle),
                0, earthScale, 0, 0,
                0, 0, earthScale, radius * sin(angle),
                0, 0, 0, 1;
            earth->Set_Model_Matrix(t);

            //// set object's material
            earth->Set_Ka(Vector3f(0.1, 0.1, 0.1));
            earth->Set_Kd(Vector3f(0.7, 0.7, 0.7));
            earth->Set_Ks(Vector3f(2, 2, 2));
            earth->Set_Shininess(128);

            //// bind texture to object
            earth->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("earth_color"));
            earth->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            earth->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //JUPITER
        {
            //// create object by reading an obj mesh
            auto jupiter = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (2.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << jupiterScale, 0, 0, radius * cos(angle),
                0, jupiterScale, 0, 0,
                0, 0, jupiterScale, radius * sin(angle),
                0, 0, 0, 1;
            jupiter->Set_Model_Matrix(t);

            //// set object's material
            jupiter->Set_Ka(Vector3f(0.2, 0.2, 0.2));
            jupiter->Set_Kd(Vector3f(0.6, 0.7, 0.8));
            jupiter->Set_Ks(Vector3f(0.3, 0.3, 0.3));
            jupiter->Set_Shininess(20);

            //// bind texture to object
            jupiter->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("jupiter_color"));
            jupiter->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            jupiter->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //VENUS
        {
            //// create object by reading an obj mesh
            auto venus = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (4.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << venusScale, 0, 0, radius * cos(angle),
                0, venusScale, 0, 0,
                0, 0, venusScale, radius * sin(angle),
                0, 0, 0, 1;
            venus->Set_Model_Matrix(t);

            //// set object's material
            venus->Set_Ka(Vector3f(0.6, 0.6, 0.4));
            venus->Set_Kd(Vector3f(0.8, 0.8, 0.5));
            venus->Set_Ks(Vector3f(0.2, 0.2, 0.2));
            venus->Set_Shininess(20);

            //// bind texture to object
            venus->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("venus_color"));
            venus->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            venus->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //MARS
        {
            //// create object by reading an obj mesh
            auto mars = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (6.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << marsScale, 0, 0, radius * cos(angle),
                0, marsScale, 0, 0,
                0, 0, marsScale, radius * sin(angle),
                0, 0, 0, 1;
            mars->Set_Model_Matrix(t);

            //// set object's material
            mars->Set_Ka(Vector3f(0.4, 0.2, 0.2));
            mars->Set_Kd(Vector3f(0.7, 0.4, 0.4));
            mars->Set_Ks(Vector3f(0.3, 0.3, 0.3));
            mars->Set_Shininess(30);

            //// bind texture to object
            mars->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("mars_color"));
            mars->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            mars->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //MERCURY
        {
            //// create object by reading an obj mesh
            auto mercury = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (8.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << mercuryScale, 0, 0, radius * cos(angle),
                0, mercuryScale, 0, 0,
                0, 0, mercuryScale, radius * sin(angle),
                0, 0, 0, 1;
            mercury->Set_Model_Matrix(t);

            //// set object's material
            mercury->Set_Ka(Vector3f(0.3, 0.3, 0.3));
            mercury->Set_Kd(Vector3f(0.5, 0.5, 0.5));
            mercury->Set_Ks(Vector3f(0.2, 0.2, 0.2));
            mercury->Set_Shininess(20);

            //// bind texture to object
            mercury->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("mercury_color"));
            mercury->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            mercury->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //URANUS
        {
            //// create object by reading an obj mesh
            auto uranus = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (10.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << uranusScale, 0, 0, radius * cos(angle),
                0, uranusScale, 0, 0,
                0, 0, uranusScale, radius * sin(angle),
                0, 0, 0, 1;
            uranus->Set_Model_Matrix(t);

            //// set object's material
            uranus->Set_Ka(Vector3f(0.2, 0.3, 0.4));
            uranus->Set_Kd(Vector3f(0.4, 0.6, 0.8));
            uranus->Set_Ks(Vector3f(0.2, 0.2, 0.2));
            uranus->Set_Shininess(20);

            //// bind texture to object
            uranus->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("uranus_color"));
            uranus->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            uranus->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //NEPTUNE
        {
            //// create object by reading an obj mesh
            auto neptune = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (12.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << neptuneScale, 0, 0, radius * cos(angle),
                0, neptuneScale, 0, 0,
                0, 0, neptuneScale, radius * sin(angle),
                0, 0, 0, 1;
            neptune->Set_Model_Matrix(t);

            //// set object's material
            neptune->Set_Ka(Vector3f(0.1, 0.1, 0.3));
            neptune->Set_Kd(Vector3f(0.2, 0.3, 0.8));
            neptune->Set_Ks(Vector3f(0.1, 0.1, 0.2));
            neptune->Set_Shininess(15);

            //// bind texture to object
            neptune->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("neptune_color"));
            neptune->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            neptune->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //SUN
        {
            //// create object by reading an obj mesh
            auto sun = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << sunScale, 0, 0, 0,
                0, sunScale, 0, 0,
                0, 0, sunScale, 0,
                0, 0, 0, 1;
            sun->Set_Model_Matrix(t);

            //// set object's material
            sun->Set_Ka(Vector3f(0.9, 0.9, 0.7));
            sun->Set_Kd(Vector3f(1., 0.9, 0.6));
            sun->Set_Ks(Vector3f(1., 0.9, 0.6));
            sun->Set_Shininess(120);

            //// bind texture to object
            sun->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("sun_color"));
            sun->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            sun->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }

        //MOON
        {
            //// create object by reading an obj mesh
            auto moon = Add_Obj_Mesh_Object("obj/sphere.obj");
            angle = (14.0f * M_PI) / p_num;

            //// set object's transform
            Matrix4f t;
            t << moonScale, 0, 0, radius * cos(angle),
                0, moonScale, 0, 0,
                0, 0, moonScale, radius * sin(angle),
                0, 0, 0, 1;
            moon->Set_Model_Matrix(t);

            //// set object's material
            moon->Set_Ka(Vector3f(0.4, 0.4, 0.4));
            moon->Set_Kd(Vector3f(0.6, 0.6, 0.6));
            moon->Set_Ks(Vector3f(0.2, 0.2, 0.2));
            moon->Set_Shininess(20);

            //// bind texture to object
            moon->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("moon_color"));
            moon->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            moon->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }



        //// Here we load a bunny object with the basic shader to show how to add an object into the scene
        /*
        {
            //// create object by reading an obj mesh
            auto bunny = Add_Obj_Mesh_Object("obj/bunny.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, 1.5,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            bunny->Set_Model_Matrix(t);

            //// set object's material
            bunny->Set_Ka(Vector3f(0.1, 0.1, 0.1));
            bunny->Set_Kd(Vector3f(0.7, 0.7, 0.7));
            bunny->Set_Ks(Vector3f(2, 2, 2));
            bunny->Set_Shininess(128);

            //// bind texture to object
            bunny->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("bunny_color"));
            bunny->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("bunny_normal"));

            //// bind shader to object
            bunny->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        */

        //// Here we show an example of adding a mesh with noise-terrain (A6)
        /*
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            //// set object's transform
            Matrix4f r, s, t;
            r << 1, 0, 0, 0,
                0, 0.5, 0.67, 0,
                0, -0.67, 0.5, 0,
                0, 0, 0, 1;
            s << 0.5, 0, 0, 0,
                0, 0.5, 0, 0,
                0, 0, 0.5, 0,
                0, 0, 0, 1;
            t << 1, 0, 0, -2,
                 0, 1, 0, 0.5,
                 0, 0, 1, 0,
                 0, 0, 0, 1,
            terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);

            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("terrain"));
        }
        */

        //// Here we show an example of adding a transparent object with alpha blending
        //// This example will be useful if you implement objects such as tree leaves, grass blades, flower pedals, etc.
        //// Alpha blending will be turned on automatically if your texture has the alpha channel
        /*
        {
            //// create object by reading an obj mesh
            auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, -0.5,
                0, 1, 0, 0,
                0, 0, 1, 1.5,
                0, 0, 0, 1;
            sqad->Set_Model_Matrix(t);

            //// bind texture to object
            sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("window_color"));

            //// bind shader to object
            sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("blend"));
        }
        */

        //// Here we show an example of adding a billboard particle with a star shape using alpha blending
        //// The billboard is rendered with its texture and is always facing the camera.
        //// This example will be useful if you plan to implement a CPU-based particle system.
        /*
        {
            //// create object by reading an obj mesh
            auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 2.5,
                 0, 0, 0, 1;
            sqad->Set_Model_Matrix(t);

            //// bind texture to object
            sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("star_color"));

            //// bind shader to object
            sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("billboard"));
        }
        */

        //// Here we show an example of shading (ray-tracing) a sphere with environment mapping
        /*
        {
            //// create object by reading an obj mesh
            auto sphere2 = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << .6, 0, 0, 0,
                0, .6, 0, -.5,
                0, 0, .6, 1,
                0, 0, 0, 1;
            sphere2->Set_Model_Matrix(t);

            //// bind shader to object
            sphere2->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("environment")); // bind shader to object
        }
        */

        //// Here we create a mesh object with two triangle specified using a vertex array and a triangle array.
        //// This is an example showing how to create a mesh object without reading an .obj file. 
        //// If you are creating your own L-system, you may use this function to visualize your mesh.
        /*
        {
            std::vector<Vector3> vertices = { Vector3(0.5, 0, 0), Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0) };
            std::vector<Vector3i> elements = { Vector3i(0, 1, 2), Vector3i(0, 2, 3) };
            auto obj = Add_Tri_Mesh_Object(vertices, elements);
            // ! you can also set uvs 
            obj->mesh.Uvs() = { Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1) };

            Matrix4f t;
            t << 1, 0, 0, -0.5,
                0, 1, 0, -1.5,
                0, 0, 1, 0,
                0, 0, 0, 1;

            obj->Set_Model_Matrix(t);

            obj->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("buzz_color"));

            obj->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        */

        //// This for-loop updates the rendering model for each object on the list
        for (auto &mesh_obj : mesh_object_array){
            Set_Polygon_Mode(mesh_obj, PolygonMode::Fill);
            Set_Shading_Mode(mesh_obj, ShadingMode::TexAlpha);
            mesh_obj->Set_Data_Refreshed();
            mesh_obj->Initialize();
        }
        Toggle_Play();
    }

    //// add mesh object by reading an .obj file
    OpenGLTriangleMesh *Add_Obj_Mesh_Object(std::string obj_file_name)
    {
        auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        Array<std::shared_ptr<TriangleMesh<3>>> meshes;
        // Obj::Read_From_Obj_File(obj_file_name, meshes);
        Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name, meshes);

        mesh_obj->mesh = *meshes[0];
        std::cout << "load tri_mesh from obj file, #vtx: " << mesh_obj->mesh.Vertices().size() << ", #ele: " << mesh_obj->mesh.Elements().size() << std::endl;

        mesh_object_array.push_back(mesh_obj);
        return mesh_obj;
    }

    //// add mesh object by reading an array of vertices and an array of elements
    OpenGLTriangleMesh* Add_Tri_Mesh_Object(const std::vector<Vector3>& vertices, const std::vector<Vector3i>& elements)
    {
        auto obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        mesh_object_array.push_back(obj);
        // set up vertices and elements
        obj->mesh.Vertices() = vertices;
        obj->mesh.Elements() = elements;

        return obj;
    }

    //// Go to next frame
    virtual void Toggle_Next_Frame()
    {
        //Update rotation angle of planets
        for (auto &mesh_obj : mesh_object_array)
            mesh_obj->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);

        if (bgEffect){
            bgEffect->setResolution((float)Win_Width(), (float)Win_Height());
            bgEffect->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
            bgEffect->setFrame(frame++);
        }

        if (skybox){
            skybox->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
        }   

        OpenGLViewer::Toggle_Next_Frame();
    }

    virtual void Run()
    {
        OpenGLViewer::Run();
    }
};

int main(int argc, char *argv[])
{
    MyDriver driver;
    driver.Initialize();
    driver.Run();
}

#endif