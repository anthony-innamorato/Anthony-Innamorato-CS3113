#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	SDL_Event event;
	bool done = false;


	glViewport(0, 0, 1280, 720);
	ShaderProgram unTextProg;
	unTextProg.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	float lastFrameTicks = 0.0f;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);


	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		//if (keys[SDL_SCANCODE_RIGHT])
		//if (keys[SDL_SCANCODE_LEFT])

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(unTextProg.programID);
		//p1
		//Matrix modelMatrix;
		//modelMatrix.Translate(-3.55, p1y + -0.5, 0.0);
		////modelMatrix.Rotate(45.0f * (3.14159265f / 180.0f));  remember to convert to radians
		//modelMatrix.Scale(.1, .1, 1.0);

		//unTextProg.SetProjectionMatrix(projectionMatrix);
		//unTextProg.SetViewMatrix(viewMatrix);
		//unTextProg.SetModelMatrix(modelMatrix);

		//float vertices[] = { 2.0, 0.0, 0.0, 10.0, 0.0, 0.0 };
		//glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		//glEnableVertexAttribArray(unTextProg.positionAttribute);

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDisableVertexAttribArray(unTextProg.positionAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
