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
#include <cmath>

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
	float p1y = 0;
	float p2y = 0;
	float ballX = 0;
	float ballY = 0;
	float ballXSpeed = -1.0;
	float ballYSpeed = -.75;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);


	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ballX += cos(45) * elapsed * ballXSpeed;
		ballY += sin(45) * elapsed * ballYSpeed;

		if (ballX <= -3.55 || ballX >= 3.55)
		{
			done = true;
		}

		//collision with bounds
		else if (ballY >= 1.6 || ballY <= -1.8)
		{
			if (ballX >= -3.0 || ballX <= 3.0)
			{
				ballYSpeed *= -1.0;
			}
		}

		//p1 hit
		else if (ballX <= -3.3)
		{
			if (ballY >= (p1y - .5) && ballY <= p1y + .5)
			{
				ballXSpeed *= -1.1;
			}
		}

		//p2 hit
		else if (ballX >= 3.1)
		{
			if (ballY >= (p2y - .5) && ballY <= p2y + .5)
			{
				ballXSpeed *= -1.1;
			}
		}

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		if (keys[SDL_SCANCODE_UP])
		{
			if (p1y <= 1.5)
			{
				p1y += elapsed * 5;
				if (p1y >= 1.5)
				{
					p1y = 1.5;
				}
			}
		}
		if (keys[SDL_SCANCODE_DOWN])
		{
			if (p1y >= -1.5)
			{
				p1y -= elapsed * 5;
				if (p1y <= -1.5)
				{
					p1y = -1.5;
				}
			}
		}
		if (keys[SDL_SCANCODE_RIGHT])
		{
			if (p2y <= 1.5)
			{
				p2y += elapsed * 5;
				if (p2y >= 1.5)
				{
					p2y = 1.5;
				}
			}
		}
		if (keys[SDL_SCANCODE_LEFT])
		{
			if (p2y >= -1.5)
			{
				p2y -= elapsed * 5;
				if (p2y <= -1.5)
				{
					p2y = -1.5;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(unTextProg.programID);


		//modelMatrix.Identity();
		//modelMatrix.Translate(1.0f, 0.0f, 0.0f);  move along axes
		//modelMatrix.Scale(2.0f, 2.0f, 1.0f);
		//modelMatrix.Rotate(45.0f * (3.14159265f / 180.0f));  remember to convert to radians
		//REMEMBER TO TRANSLATE BEFORE ROTATIONS
		//SET MODEL MATRIX FOR EACH OBJ AND SET TO IDENT

		//p1
		Matrix modelMatrix;
		modelMatrix.Translate(-3.55, p1y + -0.5, 0.0);
		modelMatrix.Scale(.1, .1, 1.0);

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix);

		float vertices[] = { 2.0, 0.0, 0.0, 10.0, 0.0, 0.0 };
		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		Matrix modelMatrix2;
		modelMatrix2.Translate(0.2 - 3.55, p1y + 0.5, 0.0);
		modelMatrix2.Rotate(180.0 * (3.14159265 / 180.0));
		modelMatrix2.Scale(.1, .1, 1.0);

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix2);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		//p2
		Matrix modelMatrix3;
		modelMatrix3.Translate(3.55-.2, p2y + -.5, 0.0);
		modelMatrix3.Scale(.1, .1, 1.0);

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix3);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		Matrix modelMatrix4;
		modelMatrix4.Translate(3.55, p2y + .5, 0.0);
		modelMatrix4.Rotate(180.0 * (3.14159265 / 180.0));
		modelMatrix4.Scale(.1, .1, 1.0);

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix4);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		//upper bound
		Matrix modelMatrix5;
		modelMatrix5.Translate(-3.0, 1.9, 0.0);
		modelMatrix5.Rotate(270.0 * (3.14159265 / 180.0));
		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix5);

		float vertices_bounds[] = { .10, 0.0, 0.0, 6.0, 0.0, 0.0 };
		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices_bounds);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		Matrix modelMatrix6;
		modelMatrix6.Translate(3.0, 1.8, 0.0);
		modelMatrix6.Rotate(90.0 * (3.14159265 / 180.0));

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix6);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices_bounds);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);


		//lower bound
		Matrix modelMatrix7;
		modelMatrix7.Translate(-3.0, -1.8, 0.0);
		modelMatrix7.Rotate(270.0 * (3.14159265 / 180.0));

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix7);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices_bounds);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		Matrix modelMatrix8;
		modelMatrix8.Translate(3.0, -1.9, 0.0);
		modelMatrix8.Rotate(90.0 * (3.14159265 / 180.0));

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix8);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices_bounds);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		//ball
		Matrix modelMatrixBall;
		modelMatrixBall.Translate(ballX, ballY, 1.0);

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrixBall);

		float verticesBall[] = { .20, 0.0, 0.0, .20, 0.0, 0.0 };
		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, verticesBall);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		
		Matrix modelMatrixBall2;
		modelMatrixBall2.Translate(ballX + .2, ballY + .2, 1.0);
		modelMatrixBall2.Rotate(180.0 * (3.14159265 / 180.0));

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrixBall2);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, verticesBall);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);
		

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
