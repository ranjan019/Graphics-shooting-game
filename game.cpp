#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
//void resetbullet();
double last_update_time = glfwGetTime(), current_time, t, powertimeend, powertimestart;
float slab=0;
float cannon_rot_angle=0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float cannon_rot_increment = 0;
int flagscore=0, newflagcannon=0, oldflagcannon=0, bulletflag=0, resetbulletflag=0;
float cannonrotflag=0;
float ux=0,uy=0, vx, vy, sx=-7, sy=-4, ax, ay,powerfac=2;
float cannon_rotation =0;
float zoom=1, a=-12.0f, b=12.0f, c=-8.0f, d=8.0f;;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset>0)
    {  if(zoom<=0.990)  
        zoom+=0.01;
        Matrices.projection = glm::ortho(a*zoom, b*zoom, c*zoom, d*zoom, 0.1f, 500.0f);
    } else{
        zoom-=0.01;
        Matrices.projection = glm::ortho(a*zoom, b*zoom, c*zoom, d*zoom, 0.1f, 500.0f);
    }}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_PRESS)
    {
        switch (key) {
            case GLFW_KEY_UP:
                if(bulletflag!=1)
                {
                    cannonrotflag=1;    
                    newflagcannon++;}
                break;
            case GLFW_KEY_DOWN:
                if(bulletflag!=1)
                {
                    cannonrotflag=-1;    
                    newflagcannon--;}
                break;
            case GLFW_KEY_SPACE:
                if(bulletflag!=1)
                    powertimestart=glfwGetTime();
                break;

        }
    }
    if (action == GLFW_RELEASE)
    {
        switch (key) {
            case GLFW_KEY_SPACE:
                if(bulletflag!=1)
                { bulletflag=1;
                    last_update_time=glfwGetTime();
                    powertimeend=glfwGetTime();
                    ux=((powertimeend-powertimestart)*powerfac)*cos(cannon_rotation*M_PI/180.0f); 
                    uy=((powertimeend-powertimestart)*powerfac)*sin(cannon_rotation*M_PI/180.0f); }
                break;
            case GLFW_KEY_UP:
                cannonrotflag=0;
                break;
            case GLFW_KEY_DOWN:
                cannonrotflag=0;
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        case 'o':
            if(zoom<=0.995)
                zoom=zoom+0.005;
            Matrices.projection = glm::ortho(a*zoom, b*zoom, c*zoom, d*zoom, 0.1f, 500.0f);
            break;
        case 'p':
            if(zoom>0.005)
                zoom=zoom-0.005;
            Matrices.projection = glm::ortho(a*zoom, b*zoom, c*zoom, d*zoom, 0.1f, 500.0f);
            break;
        case 'r':
            bulletflag=0;
            sx=-9+2*cos(cannon_rotation*M_PI/180.0f);
            sy=-4+2*sin((cannon_rotation)*M_PI/180.0f);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
            {
                if(bulletflag!=1){
                    powertimestart=glfwGetTime();}}
            else if (action == GLFW_RELEASE)
            {if(bulletflag!=1){
                                  bulletflag=1;
                                  last_update_time=glfwGetTime();
                                  powertimeend=glfwGetTime();
                                  ux=((powertimeend-powertimestart)*powerfac)*cos(cannon_rotation*M_PI/180.0f); 
                                  uy=((powertimeend-powertimestart)*powerfac)*sin(cannon_rotation*M_PI/180.0f); 
                              }}
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                bulletflag=0;
                sx=-9+2*cos(cannon_rotation*M_PI/180.0f);
                sy=-4+2*sin((cannon_rotation)*M_PI/180.0f);
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //     printf("%f\n",zoom);
    //   float a=-(12.0f)*zoom, b= (12.0f)*zoom, c=-(8.0f)*zoom, d=(8.0f)*zoom;
    Matrices.projection = glm::ortho(a*zoom, b*zoom, c*zoom, d*zoom, 0.1f, 500.0f);
}

VAO *barrier1, *barrier2, *bullet, *triangle,*rectangleleft, *rectangleright, *rectanglesideup,*rectangle, *cannon, *square1, *square2, *rectangle1, *rectangle2, *rectangle3, *square3, *square4, *square5;

// Creates the triangle object used in this sample code
void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0, 1,0, // vertex 0
        -1,-1,0, // vertex 1
        1,-1,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0.5,0, // color 0
        1,0.5,0, // color 1
        1,0.5,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -12,-1,0, // vertex 1
        -12,1,0, // vertex 2
        12,1 ,0, // vertex 3

        12, 1,0, // vertex 3
        12,-1,0, // vertex 4
        -12,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.97,0.3,0.3, // color 1
        0.97,0.3,0.3, // color 2
        0.97,0.3,0.3, // color 3

        0.97,0.3,0.3, // color 3
        0.97,0.3,0.3, // color 4
        0.97,0.3,0.3  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createSquare1 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -1,-1,0, // vertex 1
        -1,1,0, // vertex 2
        1,1 ,0, // vertex 3

        1, 1,0, // vertex 3
        1,-1,0, // vertex 4
        -1,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        1,1,0, // color 1
        1,1,0, // color 2
        1,1,0, // color 3

        1,1,0, // color 3
        1,1,0, // color 4
        1,1,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    square1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createSquare2 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -1,-1,0, // vertex 1
        -1,1,0, // vertex 2
        1,1 ,0, // vertex 3

        1, 1,0, // vertex 3
        1,-1,0, // vertex 4
        -1,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        1,1,0, // color 1
        1,1,0, // color 2
        1,1,0, // color 3

        1,1,0, // color 3
        1,1,0, // color 4
        1,1,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    square2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle1 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-1.5,0, // vertex 1
        -0.5,1.5,0, // vertex 2
        0.5,1.5 ,0, // vertex 3

        0.5, 1.5,0, // vertex 3
        0.5,-1.5,0, // vertex 4
        -0.5,-1.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.8,0.8,0, // color 1
        0.8,0.8,0, // color 2
        0.8,0.8,0, // color 3

        0.8,0.8,0, // color 3
        0.8,0.8,0, // color 4
        0.8,0.8,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle2 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-1.5,0, // vertex 1
        -0.5,1.5,0, // vertex 2
        0.5,1.5 ,0, // vertex 3

        0.5, 1.5,0, // vertex 3
        0.5,-1.5,0, // vertex 4
        -0.5,-1.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.8,0.8,0, // color 1
        0.8,0.8,0, // color 2
        0.8,0.8,0, // color 3

        0.8,0.8,0, // color 3
        0.8,0.8,0, // color 4
        0.8,0.8,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCannon()
{

    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-0.45,0, // vertex 1
        -0.5,0.45,0, // vertex 2
        2.25,0.45 ,0, // vertex 3

        2.25,0.45,0, // vertex 3
        2.25,-0.45,0, // vertex 4
        -0.5,-0.45,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.6,0.4,0, // color 1
        0.6,0.4,0, // color 2
        0.6,0.4,0, // color 3

        0.6,0.4,0, // color 3
        0.6,0.4,0, // color 4
        0.6,0.4,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle3()
{

    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -2.5,-0.5,0, // vertex 1
        -2.5,0.5,0, // vertex 2
        2.5,0.5 ,0, // vertex 3

        2.5,0.5,0, // vertex 3
        2.5,-0.5,0, // vertex 4
        -2.5,-0.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.85,0.85,0, // color 1
        0.85,0.85,0, // color 2
        0.85,0.85,0, // color 3

        0.85,0.85,0, // color 3
        0.85,0.85,0, // color 4
        0.85,0.85,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBarrier1()
{

    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.15,-1.5,0, // vertex 1
        -0.15,1.5,0, // vertex 2
        0.15,1.5 ,0, // vertex 3

        0.15,1.5,0, // vertex 3
        0.15,-1.5,0, // vertex 4
        -0.15,-1.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.4,0.8,0, // color 1
        0.4,0.8,0, // color 2
        0.4,0.8,0, // color 3

        0.4,0.8,0, // color 3
        0.4,0.8,0, // color 4
        0.4,0.8,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    barrier1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBarrier2()
{

    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.15,-1.5,0, // vertex 1
        -0.15,1.5,0, // vertex 2
        0.15,1.5 ,0, // vertex 3

        0.15,1.5,0, // vertex 3
        0.15,-1.5,0, // vertex 4
        -0.15,-1.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.4,0.8,0, // color 1
        0.4,0.8,0, // color 2
        0.4,0.8,0, // color 3

        0.4,0.8,0, // color 3
        0.4,0.8,0, // color 4
        0.4,0.8,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    barrier2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createSquare3 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -2,-1,0, // vertex 1
        -2,0.7,0, // vertex 2
        2,0.7 ,0, // vertex 3

        2, 0.7,0, // vertex 3
        2,-1,0, // vertex 4
        -2,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.98,0.98,0, // color 1
        0.98,0.98,0, // color 2
        0.98,0.98,0, // color 3

        0.98,0.98,0, // color 3
        0.98,0.98,0, // color 4
        0.98,0.98,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    square3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSquare4 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -1,-0.5,0, // vertex 1
        -1,0.5,0, // vertex 2
        1,0.5 ,0, // vertex 3

        1, 0.5,0, // vertex 3
        1,-0.5,0, // vertex 4
        -1,-0.5,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.96,0.96,0, // color 1
        0.96,0.96,0, // color 2
        0.96,0.96,0, // color 3

        0.96,0.96,0, // color 3
        0.96,0.96,0, // color 4
        0.96,0.96,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    square4 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createSquare5 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.25,-0.25,0, // vertex 1
        -0.25,0.25,0, // vertex 2
        0.25,0.25 ,0, // vertex 3

        0.25, 0.25,0, // vertex 3
        0.25,-0.25,0, // vertex 4
        -0.25,-0.25,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.94,0.94,0, // color 1
        0.94,0.94,0, // color 2
        0.94,0.94,0, // color 3

        0.94,0.94,0, // color 3
        0.94,0.94,0, // color 4
        0.94,0.94,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    square5 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBullet ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.25,-0.25,0, // vertex 1
        -0.15,0.15,0, // vertex 2
        0.25,0.25 ,0, // vertex 3

        0.15, 0.15,0, // vertex 3
        0.15,-0.15,0, // vertex 4
        -0.15,-0.15,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0,0,0, // color 4
        0,0,0  // color 1
    };


    // create3DObject creates and returns a handle to a VAO that can be used later
    bullet = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createRectanglesideup()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -12,-0.3,0, // vertex 1
        -12,0.5,0, // vertex 2
        12,0.5 ,0, // vertex 3

        12, 0.5,0, // vertex 3
        12,-0.3,0, // vertex 4
        -12,-0.3,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.97,0.3,0.3, // color 1
        0.97,0.3,0.3, // color 2
        0.97,0.3,0.3, // color 3

        0.97,0.3,0.3, // color 3
        0.97,0.3,0.3, // color 4
        0.97,0.3,0.3  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectanglesideup= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangleleft()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.4,-8,0, // vertex 1
        -0.4,8,0, // vertex 2
        0.4,8 ,0, // vertex 3

        0.4, 8,0, // vertex 3
        0.4,-8,0, // vertex 4
        -0.4,-8,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.97,0.3,0.3, // color 1
        0.97,0.3,0.3, // color 2
        0.97,0.3,0.3, // color 3

        0.97,0.3,0.3, // color 3
        0.97,0.3,0.3, // color 4
        0.97,0.3,0.3  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangleleft= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangleright()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -0.4,-8,0, // vertex 1
        -0.4,8,0, // vertex 2
        0.4,8 ,0, // vertex 3

        0.4, 8,0, // vertex 3
        0.4,-8,0, // vertex 4
        -0.4,-8,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0.97,0.3,0.3, // color 1
        0.97,0.3,0.3, // color 2
        0.97,0.3,0.3, // color 3

        0.97,0.3,0.3, // color 3
        0.97,0.3,0.3, // color 4
        0.97,0.3,0.3  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangleright= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
int flagsquare5=0,flagrectangle1=0,flagrectangle2=0,flagsquare1=0,flagsquare2=0,flagsquare3=0,flagsquare4=0,flagrectangle3=0,flagtriangle=0;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float square5_rotation = 0;
float bullet_rotation = 0;
float triangle_rotation = 0;
float barrier1_rotation=0;
float barrier2_rotation=0;

void resetprojectile()
{
    vx=ux+ax*t;
    vy=uy+ay*t;
    last_update_time=glfwGetTime();
}

int checkcollisionbarrier()
{

    float dis1=abs(sy-(tan(barrier1_rotation*M_PI/180.0f))*sx -3 -tan(barrier1_rotation*M_PI/180.0f))/sqrt(1+tan(barrier1_rotation*M_PI/180.0f)*tan(barrier1_rotation*M_PI/180.0f));

    float dis2=abs(sy-(tan(barrier2_rotation*M_PI/180.0f))*sx +3 -tan(barrier2_rotation*M_PI/180.0f))/sqrt(1+tan(barrier2_rotation*M_PI/180.0f)*tan(barrier2_rotation*M_PI/180.0f));

    float dis3=sqrt(pow((sx+1),2)+pow((sy-3),2)); 
    float dis4=sqrt(pow((sx+1),2)+pow((sy+3),2));

    if((dis1<=0.3 && dis3<=1.5) || (dis2<=0.3 && dis4<=1.5))   
    {
        return 1;
    }

}

void cannonanglecheck()
{
    if(cannonrotflag==1 && cannon_rotation<75)
    {
        cannon_rotation+=cannonrotflag;
        sx=-9+2*cos(cannon_rotation*M_PI/180.0f);
        sy=-4+2*sin((cannon_rotation)*M_PI/180.0f);
    }
    else if(cannonrotflag==-1 && cannon_rotation>-45)
    {
        cannon_rotation+=cannonrotflag;
        sx=-9+2*cos(cannon_rotation*M_PI/180.0f);
        sy=-4+2*sin(cannon_rotation*M_PI/180.0f);
    }
}
/* Render the scene with openGL */
/*Edit this function according to your assignment */

void resetbullet()
{
    cannonanglecheck();
    bulletflag=0;
}


void draw ()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */

    glm::mat4 translateTriangle = glm::translate (glm::vec3(6, -5, 0)); // glTranslatef
    //STOPPING ROTATION OF TRIANGLE
    // glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,1,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle;// * rotateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-6)<=0.7) && (abs(sy+5)<=0.9)&& flagtriangle!=1)
    {
        flagscore+=25;
        flagtriangle=1;
        resetprojectile();
        if(sx<5.3)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-4.1)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagtriangle!=1)    
        draw3DObject(triangle);

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();



    Matrices.model = glm::mat4(1.0f);
    slab=slab+0.1;
    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, -7, 0));        // glTranslatef
    Matrices.model *= (translateRectangle);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle);


    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCannon = glm::translate (glm::vec3(-9, -4, 0));        // glTranslatef
    glm::mat4 rotateCannon = glm::rotate((float)(cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateCannon* rotateCannon);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(cannon);


    cannonanglecheck();



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare1 = glm::translate (glm::vec3(4, -5, 0));        // glTranslatef
    Matrices.model *= (translateSquare1);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-4)<=1.1) && (abs(sy+5)<=1.1) && flagsquare1!=1)
    {
        flagscore+=10;
        flagsquare1=1;
        resetprojectile();
        if(sx<3)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-4.1)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagsquare1!=1)    
        draw3DObject(square1);


    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateSquare2 = glm::translate (glm::vec3(8, -5, 0));        // glTranslatef
    Matrices.model *= (translateSquare2);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-8)<=1.1) && (abs(sy+5)<=1.1) && flagsquare2!=1)
    {
        flagscore+=10;
        flagsquare2=1;
        resetprojectile();
        if(sx<7)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-4)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagsquare2!=1)    
        draw3DObject(square2);



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare3 = glm::translate (glm::vec3(6, -2, 0));        // glTranslatef
    Matrices.model *= (translateSquare3);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-6)<=2.1) && (abs(sy+2)<=0.8) && flagsquare3!=1)
    {
        flagscore+=5;
        flagsquare3=1;
        resetprojectile();
        if(sx<4.1)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-1.4)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagsquare3!=1)    
        draw3DObject(square3);



    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateSquare4 = glm::translate (glm::vec3(6, -0.8, 0));        // glTranslatef
    Matrices.model *= (translateSquare4);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    if((abs(sx-6)<=1.1) && (abs(sy+0.8)<=0.6) && flagsquare4!=1)
    {
        flagscore+=5;
        flagsquare4=1;
        resetprojectile();
        if(sx<5.01)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-0.35)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagsquare4!=1)    
        draw3DObject(square4);



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare5 = glm::translate (glm::vec3(6,0.25 , 0));        // glTranslatef
    glm::mat4 rotateSquare5 = glm::rotate((float)(square5_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare5 * rotateSquare5);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    if((abs(sx-6)<=0.35) && (abs(sy-0.25)<=0.35) &&flagsquare5!=1)
    {
        flagscore+=20;
        flagsquare5=1;
        resetprojectile();
        if(sx<5.75)
        {
            ux=-vx*(3/4);
            uy=-vy*(3/4);
            sx=sx-0.3;
        }
        else if(sy>0.50)
        {
            uy=-vy*(3/4);
            ux=-vx*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagsquare5!=1)    
        draw3DObject(square5);

    //  Increment angles
    float increments = 3;

    square5_rotation = square5_rotation + increments; //*rectangle_rot_dir*rectangle_rot_status;





    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle1 = glm::translate (glm::vec3(2.5, -4.5, 0));        // glTranslatef
    Matrices.model *= (translateRectangle1);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-2.5)<=0.6) && (abs(sy+4.5)<=1.6) && flagrectangle1!=1) 
    {
        flagscore+=3;
        flagrectangle1=1;
        resetprojectile();
        if(sx<2)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-3.1)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagrectangle1!=1)    
        draw3DObject(rectangle1);



    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle2 = glm::translate (glm::vec3(9.5, -4.5, 0));        // glTranslatef
    Matrices.model *= (translateRectangle2);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    if((abs(sx-9.5)<=0.6) && (abs(sy+4.5)<=1.6) &&flagrectangle2!=1 )
    {
        flagscore+=7;
        flagrectangle2=1;
        resetprojectile();
        if(sx<9.01)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-3.01)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagrectangle2!=1)    
        draw3DObject(rectangle2);



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateRectangle3 = glm::translate (glm::vec3(6, -3.5, 0));        // glTranslatef
    Matrices.model *= (translateRectangle3);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    if((abs(sx-6)<=2.6) && (abs(sy+3.5)<=0.6) && flagrectangle3!=1)
    {
        flagscore+=7;
        flagrectangle3=1;
        resetprojectile();
        if(sx<3.52)
        {
            ux=-vx*(3/4);
            sx=sx-0.3;
        }
        else if(sy>-3.01)
        {
            uy=-vy*(3/4);
            sy=sy+0.3;
        }
    }
    if(flagrectangle3!=1)    
        draw3DObject(rectangle3);




    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateBarrier1 = glm::translate (glm::vec3(-1, 3, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    glm::mat4 rotateBarrier1 = glm::rotate((float)(barrier1_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ((translateBarrier1)* rotateBarrier1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(barrier1);

    // Increment angles
    float barrier1increments = 2;

    //camera_rotation_angle++; // Simulating camera rotation
    barrier1_rotation = barrier1_rotation - barrier1increments;
    // printf("%f\n",barrier1_rotation); 



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateBarrier2 = glm::translate (glm::vec3(-1, -3, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    glm::mat4 rotateBarrier2 = glm::rotate((float)(barrier2_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *=((translateBarrier2) * rotateBarrier2);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(barrier2);

    // Increment angles
    float barrier2increments = 2;

    //camera_rotation_angle++; // Simulating camera rotation
    barrier2_rotation = barrier2_rotation + barrier2increments;





    vx=ux+ax*t;
    vy=uy+ay*t;

    if(resetbulletflag==1)
        resetbullet();
    if(vx<0.01)
        ax=0;
    current_time = glfwGetTime();

    t= current_time - last_update_time;
    t=t/5;
    if(bulletflag==1)
    {
        sx= sx+  ux*t+(0.5)*ax*t*t;
        sy= sy+ uy*t+(0.5)*ay*t*t;
    }
    int mmm=checkcollisionbarrier();

    if(mmm==1)
    {
        sx=sx-0.3;
        if((sy>=-3 && sy<=-1.5) || (sy >=3 && sy<=4.5))
            uy+=0.5;
        else if((sy<-3 && sy>=-4.5) || (sy <3 && sy >=1.5))
            uy-=0.5;
        resetprojectile();
        ux=-vx;
        mmm=0;
    }


    if(sy<=-5.9)
    {
        resetprojectile();
        vx=ux+ax*t;
        vy=uy+ay*t;
        sy=-6+0.12;
        uy=-(vy*(1.0))/4;
        ux=vx*3/5;
    }
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBullet = glm::translate (glm::vec3(sx,sy , 0));        // glTranslatef
    glm::mat4 rotateBullet = glm::rotate((float)(bullet_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ((translateBullet * rotateBullet));
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    draw3DObject(bullet);

    float increments2 = 100;


    //camera_rotation_angle++; // Simulating camera rotation
    bullet_rotation = bullet_rotation + increments2; //*rectangle_rot_dir*rectangle_rot_status;




    Matrices.model = glm::mat4(1.0f);
    slab=slab+0.1;
    glm::mat4 translateRectanglesideup = glm::translate (glm::vec3(0, 7.5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    Matrices.model *= (translateRectanglesideup);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectanglesideup);



    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangleleft = glm::translate (glm::vec3(-11.6, 0, 0));        // glTranslatef
    Matrices.model *= (translateRectangleleft);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangleleft);



    Matrices.model = glm::mat4(1.0f);
    //   slab=slab+0.1;
    glm::mat4 translateRectangleright = glm::translate (glm::vec3(11.6, 0, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    Matrices.model *= (translateRectangleright);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangleright);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createRectangle ();
    createCannon ();
    createSquare1 ();
    createSquare2 ();
    createSquare3 ();
    createSquare4 ();
    createSquare5 ();
    createRectangle1 ();
    createRectangle2 ();
    createRectangle3 ();
    createBullet();
    createBarrier1();
    createBarrier2();
    createRectanglesideup();
    createRectangleleft();
    createRectangleright();

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0, 1, 1, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    float gravityvariable, airvar;
    int k=0;
    int width = 900;
    int height = 600;
    printf("\n\n\n**********\nAbout the game: Shoot the cannon ball to destroy the building avoiding the obstacles.\n");
    printf("Read the help.pdf file for RULES and CONTROLS.\n**********\n");
    printf("\n\n");
    printf("|Where would you like to play the game?|\n");
    printf("|Input 1 for EARTH and 2 for MOON.|\n");
    scanf("%f",&gravityvariable);
    printf("\n");
    printf("\n|What do you want the air-resistance to be?|\n");
    printf("|Input 1 for LOW, 2 for MEDIUM and 3 for HIGH|\n");
    scanf("%f",&airvar);

    if(gravityvariable==1)
        ay=-15;
    else
        ay=-5;
    if(airvar==1)
        ax=-1;
    else if(airvar==2)
        ax=-4;
    else 
        ax=-8;

    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);



    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();
        if(flagscore>k)
        {
            printf("Score-update:%d\n",flagscore);
            k=flagscore;
            if(flagscore>90)
                printf("\nYOU WON!!\n");
        }



        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
        glfwSetScrollCallback(window, scroll_callback);

    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
