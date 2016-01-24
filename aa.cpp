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

double last_update_time = glfwGetTime(), current_time, t, powertimeend, powertimestart;
float slab=0;
float cannon_rot_angle=0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float cannon_rot_increment = 0;
int  newflagcannon=0, oldflagcannon=0, bulletflag=0;;
float cannonrotflag=0;
float ux=0,uy=0, vx=0, vy=0, sx=-7, sy=-4, ax=-2, ay=-8;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_PRESS)
    {
        switch (key) {
            case GLFW_KEY_UP:
                cannonrotflag=1;    
                newflagcannon++;
                break;
            case GLFW_KEY_DOWN:
                cannonrotflag=-1;    
                newflagcannon--;
                break;
            case GLFW_KEY_SPACE:
                powertimestart=glfwGetTime();
                break;

        }
    }
    if (action == GLFW_RELEASE)
    {
        switch (key) {
            case GLFW_KEY_SPACE:
                bulletflag=1;
                last_update_time=glfwGetTime();
                powertimeend=glfwGetTime();
                ux=(powertimeend-powertimestart)*5; 
               uy=(powertimeend-powertimestart)*3;
                break;
            case GLFW_KEY_UP:
                cannonrotflag=0;
                break;
            case GLFW_KEY_DOWN:
                cannonrotflag=0;
                break;
        }
    }
    /*    if (action == GLFW_RELEASE) {
          switch (key) {
          case GLFW_KEY_C:
          rectangle_rot_status = !rectangle_rot_status;
          break;
          case GLFW_KEY_P:
          triangle_rot_status = !triangle_rot_status;
          break;
          case GLFW_KEY_X:
    // do something ..
    break;
    default:
    break;
    }
    }
    else if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
    quit(window);
    break;
    default:
    break;
    }
    }*/
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
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
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
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
    Matrices.projection = glm::ortho(-12.0f, 12.0f, -8.0f, 8.0f, 0.1f, 500.0f);
}

VAO *barrier1, *barrier2, *bullet, *triangle, *rectangle, *cannon, *square1, *square2, *rectangle1, *rectangle2, *rectangle3, *square3, *square4, *square5;

// Creates the triangle object used in this sample code
void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0, 1,0, // vertex 0
        //  -12,-6,0, // vertex 1
        // -12,-8,0, // vertex 2
        //  12,-8 ,0, // vertex 3
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
        1,0,0, // color 1
        1,0,0, // color 2
        1,0,0, // color 3

        1,0,0, // color 3
        1,0,0, // color 4
        1,0,0  // color 1
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
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float square5_rotation = 0;
float bullet_rotation = 0;
float triangle_rotation = 0;
float cannon_rotation =0;
float barrier1_rotation=0;
float barrier2_rotation=0;

void cannonanglecheck()
{
    if(cannonrotflag==1 && cannon_rotation<45)
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
/* Edit this function according to your assignment */
void draw ()
{
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
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
    draw3DObject(triangle);

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();



    Matrices.model = glm::mat4(1.0f);
    slab=slab+0.1;
    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, -7, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    // rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;




    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateCannon = glm::translate (glm::vec3(-9, -4, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    glm::mat4 rotateCannon = glm::rotate((float)(cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateCannon* rotateCannon);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(cannon);

    // Increment angles
    //  float increments = 1;

    cannonanglecheck();


  /*  if(newflagcannon<oldflagcannon)
    {
        cannon_rot_increment=1;
        if(cannon_rot_angle>0)
        {
            sx= sx + 0.02222222222;
            sy= sy - 0.02222222222;
        }
        else
        {
            sx= sx- 0.02222222222;
            sy= sy- 0.02222222222;
        }
    }
    else if(newflagcannon>oldflagcannon)
    {

        cannon_rot_increment=-1;
        if(cannon_rot_angle>=0)
        {
            sx= sx- 0.02222222222;
            sy= sy+ 0.02222222222;
        }
        else
        {
            sx= sx+0.022222222222;
            sy= sy+0.022222222222;
        }
    }
    else
        cannon_rot_increment=0;
*/
  //  oldflagcannon=newflagcannon;
    //camera_rotation_angle++; // Simulating camera rotation
//    cannon_rotation = cannon_rotation;// - cannon_rot_increment;




    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare1 = glm::translate (glm::vec3(4, -5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare1);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(square1);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare2 = glm::translate (glm::vec3(8, -5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare2);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(square2);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare3 = glm::translate (glm::vec3(6, -2, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare3);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(square3);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare4 = glm::translate (glm::vec3(6, -0.8, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare4);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(square4);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateSquare5 = glm::translate (glm::vec3(6,0.25 , 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    glm::mat4 rotateSquare5 = glm::rotate((float)(square5_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateSquare5 * rotateSquare5);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(square5);

    //  Increment angles
    float increments = 3;

    //camera_rotation_angle++; // Simulating camera rotation
    square5_rotation = square5_rotation + increments; //*rectangle_rot_dir*rectangle_rot_status;


    //slab=slab+0.1;
    
    current_time = glfwGetTime();
    t= current_time - last_update_time;
    if(bulletflag==1)
    {
    sx= sx+  ux*t+(0.5)*ax*t*t;
    sy= sy+ uy*t+(0.5)*ay*t*t;
    }
   // last_update_time=current_time;
//    bulletflag=0;
    
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBullet = glm::translate (glm::vec3(sx,sy , 0));        // glTranslatef
    //translateBullet *= glm::translate (glm::vec3(sx,sy , 0));        // glTranslatef
    glm::mat4 rotateBullet = glm::rotate((float)(bullet_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ((translateBullet * rotateBullet));
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(bullet);

    //  Increment angles
    float increments2 = 100;

    //camera_rotation_angle++; // Simulating camera rotation
    bullet_rotation = bullet_rotation + increments2; //*rectangle_rot_dir*rectangle_rot_status;




    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateRectangle1 = glm::translate (glm::vec3(2.5, -4.5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle1);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle1);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateRectangle2 = glm::translate (glm::vec3(9.5, -4.5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle2);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle2);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



    Matrices.model = glm::mat4(1.0f);
    //slab=slab+0.1;
    glm::mat4 translateRectangle3 = glm::translate (glm::vec3(6, -3.5, 0));        // glTranslatef
    //STOPPING ROTATION OF RECTANGLE
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle3);// * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle3);

    // Increment angles
    //  float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    // cannon_rotation = cannon_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
    
    
    
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
    int width = 900;
    int height = 600;

    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

//    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
/*        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.001 && bulletflag==1) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
       t=current_time-last_update_time;   
    sx=ux*t+(1/2)*ax*t*t;
    sy=uy*t+(1/2)*ay*t*t;
            last_update_time = current_time;
  */          
    //    }

 /*   current_time = glfwGetTime();
    t= current_time - last_update_time;
    if(bulletflag==1)
    {
    sx=ux*t+(1/2)*ax*t*t;
    sy=uy*t+(1/2)*ay*t*t;
    }
    bulletflag=0;*/
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
