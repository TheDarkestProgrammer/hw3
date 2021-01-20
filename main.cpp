#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "parser.h"

struct Vertex {
    glm::vec3 position;

    Vertex(const parser::Vec3f &rhs): position(glm::vec3(rhs.x , rhs.y , rhs.z)) {}
};

//////-------- Global Variables -------/////////

GLuint vertex_array;
GLuint vertex_buffer;
GLuint gpuNormalBuffer;
GLuint gpuIndexBuffer;

// Sample usage for reading an XML scene file
parser::Scene scene;
static GLFWwindow* win = NULL;

void calc_normals(parser::Mesh &mesh){
    mesh.normals = new GLfloat[scene.vertex_data.size() * 3];

    for(auto i = 0 ; i < scene.vertex_data.size() * 3 ; ++i) mesh.normals[i] = 0 ;

    std::vector<unsigned int> table(scene.vertex_data.size(),0);
    std::vector<glm::vec3> normals(mesh.faces.size());

    unsigned i = 0;
    for(const auto &face : mesh.faces){
        normals[i] = glm::normalize(glm::cross(glm::vec3(scene.vertex_data[face.v1_id-1].x - scene.vertex_data[face.v0_id-1].x,
                            scene.vertex_data[face.v1_id - 1].y - scene.vertex_data[face.v0_id-1].y,
                            scene.vertex_data[face.v1_id - 1].z - scene.vertex_data[face.v0_id-1].z) ,
                   glm::vec3(scene.vertex_data[face.v2_id - 1].x - scene.vertex_data[face.v1_id-1].x,
                            scene.vertex_data[face.v2_id - 1].y - scene.vertex_data[face.v1_id-1].y,
                            scene.vertex_data[face.v2_id - 1].z - scene.vertex_data[face.v1_id-1].z)));

        ++i;
    }

    for(auto i = 0 ; i < mesh.faces.size() ; ++i){
        auto &face = mesh.faces[i];

        table[face.v0_id - 1] += 1;
        mesh.normals[(face.v0_id - 1) * 3    ] += normals[i].x;
        mesh.normals[(face.v0_id - 1) * 3 + 1] += normals[i].y;
        mesh.normals[(face.v0_id - 1) * 3 + 2] += normals[i].z;

        table[face.v1_id - 1] += 1;
        mesh.normals[(face.v1_id - 1) * 3    ] += normals[i].x;
        mesh.normals[(face.v1_id - 1) * 3 + 1] += normals[i].y;
        mesh.normals[(face.v1_id - 1) * 3 + 2] += normals[i].z;

        table[face.v2_id - 1] += 1;
        mesh.normals[(face.v2_id - 1) * 3    ] += normals[i].x;
        mesh.normals[(face.v2_id - 1) * 3 + 1] += normals[i].y;
        mesh.normals[(face.v2_id - 1) * 3 + 2] += normals[i].z;
    }

    for(auto i = 0 ; i < scene.vertex_data.size() ; ++i){
        if(table[i]){
            mesh.normals[i * 3    ] = mesh.normals[i * 3    ]  / table[i];
            mesh.normals[i * 3 + 1] = mesh.normals[i * 3 + 1]  / table[i];
            mesh.normals[i * 3 + 2] = mesh.normals[i * 3 + 2]  / table[i];

            double len = std::sqrt(mesh.normals[i*3] * mesh.normals[i*3] +
                                    mesh.normals[i*3+1] * mesh.normals[i*3+1] +
                                    mesh.normals[i*3+2] * mesh.normals[i*3+2]);

            if(len){
                mesh.normals[i * 3    ] /= len;
                mesh.normals[i * 3 + 1] /= len;
                mesh.normals[i * 3 + 2] /= len;
            }
        }
    }
}

void load_mesh(parser::Mesh &mesh){
    GLfloat *vertex_buff = new GLfloat[scene.vertex_data.size() * 3];
    GLuint *index_buff = new GLuint[mesh.faces.size() * 3];

    unsigned i = 0;
    for(const auto &face : mesh.faces){
        index_buff[i    ] = face.v0_id - 1;
        index_buff[i + 1] = face.v1_id - 1;
        index_buff[i + 2] = face.v2_id - 1;

        i += 3;
    }

    i = 0;
    for(const auto &vert : scene.vertex_data){
        vertex_buff[i    ] = vert.x;
        vertex_buff[i + 1] = vert.y;
        vertex_buff[i + 2] = vert.z;

        i += 3;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glGenBuffers(1, &mesh.vertexAttribBuffer);
    glGenBuffers(1, &mesh.indexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);

    auto vertexPosDataSizeInBytes = scene.vertex_data.size() * 3 * sizeof(GLfloat);
    int vertexColDataSizeInBytes = sizeof(GLfloat) * scene.vertex_data.size() * 3;
    int indexDataSizeInBytes = sizeof(GLuint) * mesh.faces.size() * 9;

    glBufferData(GL_ARRAY_BUFFER, vertexPosDataSizeInBytes + vertexColDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexPosDataSizeInBytes, vertex_buff);
    glBufferSubData(GL_ARRAY_BUFFER, vertexPosDataSizeInBytes, vertexColDataSizeInBytes, mesh.normals);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, index_buff , GL_STATIC_DRAW);

    delete[] mesh.normals;
    delete[] vertex_buff;
    delete[] index_buff;
}
/*
void load_mesh(parser::Mesh &mesh){
    for(const auto &face : mesh.faces){
        mesh.indices.push_back(face.v0_id - 1);
        mesh.indices.push_back(face.v1_id - 1);
        mesh.indices.push_back(face.v2_id - 1);
    }

    mesh.buffer_size = mesh.faces.size() ;
    mesh.faces.clear();


    glGenBuffers(1, &mesh.index_buffer);

    //glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER , mesh.indices.size() * sizeof(int) , &mesh.indices[0] , GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) , (void*)0);


    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(int) * 3 , (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(int) * 3 , (void*)0);

    //glBindVertexArray(0);

    mesh.indices.clear();
}
*/
static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void turnOnLights() {
    glEnable(GL_LIGHTING);

    GLfloat ambient[] = {scene.ambient_light.x ,scene.ambient_light.y , scene.ambient_light.z,1.0f}; //sondaki 1.0 ekli değildi
    GLfloat zero[] = {0,0,0,1};
    //glEnable(GL_LIGHT0);  //bu yoktu
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, zero);

    unsigned i = 1;
    for (const auto &light : scene.point_lights) {

        glEnable(GL_LIGHT0 + i);

        GLfloat col[] = {light.intensity.x  , light.intensity.y  , light.intensity.z  , 1.0f};
        GLfloat pos[] = {light.position.x  , light.position.y   , light.position.z    , 1.0f};


        glLightfv(GL_LIGHT0 + i, GL_POSITION, pos);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, col);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, col);
        glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambient);

        i += 1;
    }
}

double lastTime;
int nbFrames;

void showFPS(GLFWwindow *pWindow)
{
    // Measure speed
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    char ss[500] = {};
    nbFrames++;
    if ( delta >= 1.0 ){ // If last cout was more than 1 sec ago

        double fps = ((double)(nbFrames)) / delta;

        sprintf(ss,"CENG477 - HW3 %lf FPS",fps);

        glfwSetWindowTitle(pWindow, ss);

        nbFrames = 0;
        lastTime = currentTime;
    }
}

int main(int argc, char* argv[]) {
    scene.loadFromXml(argv[1]);

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    win = glfwCreateWindow(scene.camera.image_width, scene.camera.image_height, "CENG477 - HW3", NULL, NULL);
    if (!win) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(win, keyCallback);


    glEnable(GL_DEPTH_TEST);
    
    if(scene.culling_enabled){
        glEnable(GL_CULL_FACE);

        if(scene.culling_face) glCullFace(GL_FRONT);
    }

        for(auto &mesh : scene.meshes){
            calc_normals(mesh);
            load_mesh(mesh);
        }



    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    turnOnLights(); 
    gluLookAt(scene.camera.position.x ,
                scene.camera.position.y ,
                scene.camera.position.z,
                scene.camera.position.x + scene.camera.gaze.x ,
                scene.camera.position.y + scene.camera.gaze.y ,
                scene.camera.position.z + scene.camera.gaze.z ,
                scene.camera.up.x ,
                scene.camera.up.y ,
                scene.camera.up.z);
   // turnOnLights();
    
    //gluLookAt(5,3,11,  -4,-200,-1  , 0,1,0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(35, 2, 1, 300);
    glFrustum(scene.camera.near_plane.x,
                scene.camera.near_plane.y,
                scene.camera.near_plane.z,
                scene.camera.near_plane.w,
                scene.camera.near_distance,
                scene.camera.far_distance);

    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);



    lastTime = glfwGetTime();
	nbFrames = 0;
    //turnOnLights();

     int flag = 1;
    while(!glfwWindowShouldClose(win) && flag) {
        // glfwWaitEvents();

        glfwPollEvents();

        glClearColor(scene.background_color.x ,  scene.background_color.y , scene.background_color.z , 1);
        glClearDepth(1.0f);
        glClearStencil(0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        for(auto &mesh : scene.meshes){
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexAttribBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);

            if(mesh.mesh_type == "Solid") glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glPushMatrix();

            /*for(const auto &trans : mesh.transformations){
                if(trans.transformation_type == "Translation"){
                    glTranslatef(scene.translations[trans.id - 1].x , scene.translations[trans.id - 1].y , scene.translations[trans.id - 1].z);
                }else if(trans.transformation_type == "Scaling"){
                    glScalef(scene.scalings[trans.id - 1].x , scene.scalings[trans.id - 1].y , scene.scalings[trans.id - 1].z);
                }else if(trans.transformation_type == "Rotation"){
                    glRotatef(scene.rotations[trans.id - 1].x , scene.rotations[trans.id - 1].y ,  scene.rotations[trans.id - 1].z , scene.rotations[trans.id - 1].w);
                }
            }*/

            for(auto i = mesh.transformations.rbegin() ; i != mesh.transformations.rend() ; ++i){
                const auto &trans = *i;

                if(trans.transformation_type == "Translation"){
                    glTranslatef(scene.translations[trans.id - 1].x , scene.translations[trans.id - 1].y , scene.translations[trans.id - 1].z);
                }else if(trans.transformation_type == "Scaling"){
                    glScalef(scene.scalings[trans.id - 1].x , scene.scalings[trans.id - 1].y , scene.scalings[trans.id - 1].z);
                }else if(trans.transformation_type == "Rotation"){
                    glRotatef(scene.rotations[trans.id - 1].x , scene.rotations[trans.id - 1].y ,  scene.rotations[trans.id - 1].z , scene.rotations[trans.id - 1].w);
                }
            }

            glVertexPointer(3, GL_FLOAT, 0, 0);
	        glNormalPointer(GL_FLOAT, 0, reinterpret_cast<void*>(scene.vertex_data.size()*sizeof(GLfloat)*3));


            GLfloat ambColor[4] = {scene.materials[mesh.material_id - 1].ambient.x ,
                                        scene.materials[mesh.material_id - 1].ambient.y ,
                                        scene.materials[mesh.material_id - 1].ambient.z,1.0} ;
            GLfloat diffColor[4] = {scene.materials[mesh.material_id - 1].diffuse.x ,
                                        scene.materials[mesh.material_id - 1].diffuse.y ,
                                        scene.materials[mesh.material_id - 1].diffuse.z,1.0} ;
            GLfloat specColor[4] = {scene.materials[mesh.material_id - 1].specular.x ,
                                        scene.materials[mesh.material_id - 1].specular.y ,
                                        scene.materials[mesh.material_id - 1].specular.z,1.0} ;
            GLfloat specExp[1] = {scene.materials[mesh.material_id - 1].phong_exponent} ;


            glMaterialfv ( GL_FRONT_AND_BACK , GL_AMBIENT   , ambColor ) ;
            glMaterialfv ( GL_FRONT_AND_BACK , GL_DIFFUSE   , diffColor) ;
            glMaterialfv ( GL_FRONT_AND_BACK , GL_SPECULAR  , specColor) ;
            glMaterialfv ( GL_FRONT_AND_BACK , GL_SHININESS , specExp  ) ;

	        glDrawElements(GL_TRIANGLES, mesh.faces.size() * 3 , GL_UNSIGNED_INT, 0);

            glPopMatrix();
        }
        showFPS(win);
        glfwSwapBuffers(win);
    }

    for(auto &mesh : scene.meshes){
        glDeleteBuffers(1 , &mesh.vertexAttribBuffer);
        glDeleteBuffers(1 , &mesh.indexBuffer);
    }

    glfwDestroyWindow(win);
    glfwTerminate();

    exit(EXIT_SUCCESS);

    return 0;
}
