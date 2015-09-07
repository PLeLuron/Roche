#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

void read_file(char* filename, char** buffer_ptr)
{
	long length;
	FILE * f = fopen (filename, "rb");

	if (f)
	{
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  fseek (f, 0, SEEK_SET);
	  char *buffer = malloc (length+1);
	  if (buffer)
	  {
	    fread (buffer, 1, length, f);
	    buffer[length] = 0;
	  }
	  fclose (f);
	  *buffer_ptr = buffer;
	}
}

void load_shaders(GLuint *program, const char* vert_source, const char* frag_source)
{
	GLuint vertex_id, fragment_id;
    GLint success;
    GLchar infoLog[512];
    
    *program = glCreateProgram();

    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vert_source, NULL);
    glCompileShader(vertex_id);
    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_id, 512, NULL, infoLog);
        fprintf(stderr,"VERTEX SHADER FAILED TO COMPILE :\n%s\n", infoLog);
    }
    
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &frag_source, NULL);
    glCompileShader(fragment_id);
    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_id, 512, NULL, infoLog);
        fprintf(stderr,"FRAGMENT SHADER FAILED TO COMPILE :\n%s\n", infoLog);
    }
    
    
    glAttachShader(*program, vertex_id);
    glAttachShader(*program, fragment_id);

    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(*program, 512, NULL, infoLog);
        fprintf(stderr,"SHADER PROGRAM FAILED TO LINK :\n%s\n", infoLog);
    }
    
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
}

void generate_rings(unsigned char *buffer, int size, int seed)
{
    srand(seed);
    int gapsize = rand()%10 + 10;
    int gap = rand()%(size-gapsize);
    int i;
    for (i=0;i<gap;++i)
    {
        buffer[i] = 255;
    }
    for (i=gap;i<gap+gapsize;++i)
    {
        buffer[i] = 0;
    }
    for (i=gap+gapsize;i<size;++i)
    {
        buffer[i] = 255;
    }
}

int main(void)
{

    if (!glfwInit())
        return -1;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Roche", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
    	fprintf(stderr,"Some shit happened: %s\n", glewGetErrorString(err));
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float) height;
    glViewport(0, 0, width, height);
    
    GLuint program;
    char *vert_source, *frag_source;
    read_file("planet.vert", &vert_source);
    read_file("planet.frag", &frag_source);
    load_shaders(&program, vert_source, frag_source);
    free(vert_source);
    free(frag_source);

    
    const int ringsize = 2048;
    unsigned char *rings = malloc(ringsize);
    generate_rings(rings, ringsize, 1909802985);
    GLuint ring_tex;
    glGenTextures(1, &ring_tex);
    glBindTexture(GL_TEXTURE_1D, ring_tex);
    glTexImage1D(GL_TEXTURE_1D, 0, 1, ringsize,0, GL_RED,GL_UNSIGNED_BYTE, rings);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_1D, 0);
    int escape_key;

    float ring_pos[] = 
    {-1.0,-1.0,0.0,1.0,-1.0,-1.0,
     +1.0,-1.0,0.0,1.0,+1.0,-1.0,
     +1.0,+1.0,0.0,1.0,+1.0,+1.0,
     +1.0,+1.0,0.0,1.0,+1.0,+1.0,
     -1.0,+1.0,0.0,1.0,-1.0,+1.0,
     -1.0,-1.0,0.0,1.0,-1.0,-1.0};

    GLuint vbo;
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ring_pos),ring_pos,GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
		escape_key = glfwGetKey(window, GLFW_KEY_ESCAPE);
		if (escape_key == GLFW_PRESS) break;
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindTexture(GL_TEXTURE_1D, ring_tex);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,24,(GLvoid*)0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,24,(GLvoid*)16);
        glDrawArrays(GL_TRIANGLES, 0,6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_1D, 0);
        glUseProgram(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &ring_tex);
    glDeleteBuffers(1, &vbo);
    glfwTerminate();
    return 0;
}