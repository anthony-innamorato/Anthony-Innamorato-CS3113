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
#include <math.h>

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL)
	{
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640 * 1.5, 360 * 1.5, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	SDL_Event event;
	bool done = false;


	glViewport(0, 0, 640 * 1.5, 360 * 1.5);
	ShaderProgram unTextProg;
	unTextProg.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	ShaderProgram textProg;
	textProg.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");         
	GLuint markImage = LoadTexture(RESOURCE_FOLDER"shadowmark.png");
	GLuint creedImage = LoadTexture(RESOURCE_FOLDER"creed.png");
	GLuint legendaryImage = LoadTexture(RESOURCE_FOLDER"legendary.png");

	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;
	float angle = 45.0f;
	float spinMultiplier = .1;
	float xVal = 1;
	float xMult = 0;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
				{
					spinMultiplier += 10;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
				{
					xMult -= .01;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
				{
					xMult += .01;
				}
			}
		}

		glClearColor(.064, .84, .88, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(unTextProg.programID);
		unTextProg.SetColor(1.0, .0, .0, 1.0);

		float ticks = float(SDL_GetTicks() / 1000.0f);
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		angle += elapsed * spinMultiplier;
		spinMultiplier += .01;
		xVal += elapsed * xMult;


		//modelMatrix.Identity();
		//modelMatrix.Translate(1.0f, 0.0f, 0.0f);  move along axes
		//modelMatrix.Scale(2.0f, 2.0f, 1.0f);
		//modelMatrix.Rotate(45.0f * (3.14159265f / 180.0f));  remember to convert to radians
		//REMEMBER TO TRANSLATE BEFORE ROTATIONS
		//SET MODEL MATRIX FOR EACH OBJ AND SET TO IDENT

		//first triangle
		Matrix modelMatrix;
		modelMatrix.Translate(0.0f, .5f, 0.0f);
		modelMatrix.Scale(0.5f, 0.5f, 0.0f);
		modelMatrix.Rotate(angle * (3.14159265f / 180.0f));

		unTextProg.SetProjectionMatrix(projectionMatrix);
		unTextProg.SetViewMatrix(viewMatrix);
		unTextProg.SetModelMatrix(modelMatrix);

		float vertices[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		//second triangle
		Matrix modelMatrix1;
		modelMatrix1.Translate(.25f, 0.0f, 0.0f);
		modelMatrix1.Scale(0.5f, 0.5f, 0.0f);
		modelMatrix1.Rotate(angle * (3.14159265f / 180.0f));
		unTextProg.SetModelMatrix(modelMatrix1);


		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);


		//third triangle
		Matrix modelMatrix2;
		modelMatrix2.Translate(-.25f, 0.0f, 0.0f);
		modelMatrix2.Scale(0.5f, 0.5f, 0.0f);
		modelMatrix2.Rotate(angle * (3.14159265f / 180.0f));
		unTextProg.SetModelMatrix(modelMatrix2);

		glVertexAttribPointer(unTextProg.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(unTextProg.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(unTextProg.positionAttribute);

		//shadowmark
		glUseProgram(textProg.programID);
		Matrix modelMatrix3;
		modelMatrix3.Translate(xVal, 0.0f, 0.0f);
		textProg.SetModelMatrix(modelMatrix3);
		textProg.SetProjectionMatrix(projectionMatrix);
		textProg.SetViewMatrix(viewMatrix);

		
		glBindTexture(GL_TEXTURE_2D, markImage);
		float text_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(textProg.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
		glEnableVertexAttribArray(textProg.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(textProg.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textProg.texCoordAttribute);   
		glDrawArrays(GL_TRIANGLES, 0, 6);      
		glDisableVertexAttribArray(textProg.positionAttribute);    
		glDisableVertexAttribArray(textProg.texCoordAttribute);

		//creed symbol
		Matrix modelMatrix4;
		modelMatrix4.Translate(-2.0f, 0.0f, 0.0f);
		textProg.SetModelMatrix(modelMatrix4);
		textProg.SetProjectionMatrix(projectionMatrix);
		textProg.SetViewMatrix(viewMatrix);


		glBindTexture(GL_TEXTURE_2D, creedImage);
		glVertexAttribPointer(textProg.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
		glEnableVertexAttribArray(textProg.positionAttribute);

		glVertexAttribPointer(textProg.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textProg.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(textProg.positionAttribute);
		glDisableVertexAttribArray(textProg.texCoordAttribute);

		//legendary
		Matrix modelMatrix5;
		modelMatrix5.Translate(0.0f, -1.0f, 0.0f);
		textProg.SetModelMatrix(modelMatrix5);
		textProg.SetProjectionMatrix(projectionMatrix);
		textProg.SetViewMatrix(viewMatrix);


		glBindTexture(GL_TEXTURE_2D, legendaryImage);
		glVertexAttribPointer(textProg.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
		glEnableVertexAttribArray(textProg.positionAttribute);

		glVertexAttribPointer(textProg.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textProg.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(textProg.positionAttribute);
		glDisableVertexAttribArray(textProg.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
