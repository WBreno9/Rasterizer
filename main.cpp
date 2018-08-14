#include "main.h"

//-----------------------------------------------------------------------------
void MyGlDraw(void)
{
	//*************************************************************************
	// Chame aqui as funções do mygl.h
	//*************************************************************************
    
    ClearBuffer(glm::vec4(1.0));

    Vertex v0 = {}, v1 = {}, v2 = {};

    v0.pos   = glm::vec3(0.0, 0.8, 0.0);
    v0.color = glm::vec4(0.0, 0.0, 1.0, 1.0);

    v1.pos   = glm::vec3(-0.8, -0.8, 0.0);
    v1.color = glm::vec4(1.0, 0.0, 0.0, 1.0);

    v2.pos   = glm::vec3(0.8, -0.5, 0.0);
    v2.color = glm::vec4(0.0, 1.0, 0.0, 1.0);

    DrawTriangleFill(v0, v1, v2);

    /*
    Vertex v0 = {}, v1 = {}, v2 = {};

    v0.pos   = glm::vec3(0.0, 0.8, 0.0);
    v0.color = glm::vec4(0.0, 0.0, 1.0, 1.0);

    v1.pos   = glm::vec3(0.8, -0.5, 0.0);
    v1.color = glm::vec4(1.0, 0.0, 0.0, 1.0);

    v2.pos   = glm::vec3(-0.65, -0.5, 0.0);
    v2.color = glm::vec4(0.0, 1.0, 0.0, 1.0);

    DrawTriangle(v0, v1, v2);
    */

}


//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	// Inicializações.
	InitOpenGL(&argc, argv);
	InitCallBacks();
	InitDataStructures();

	// Ajusta a função que chama as funções do mygl.h
	DrawFunc = MyGlDraw;	

	// Framebuffer scan loop.
	glutMainLoop();

	return 0;
}

