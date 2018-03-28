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

#include "FlareMap.h"
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <string>
using namespace std;

class Entity;

SDL_Window* displayWindow;
ShaderProgram textured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
SDL_Event event;
float lastFrameTicks = 0.0f;
std::vector<Entity*> entities;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
FlareMap flare;
float TILE_SIZE = 16.0;
int LEVEL_HEIGHT = 16;
int LEVEL_WIDTH = 32;
int SPRITE_COUNT_X = 16;
int SPRITE_COUNT_Y = 8;
GLuint spriteSheet;

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

struct Vector
{
	Vector() {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;
};

void Setup()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 1280, 720);
	//projectionMatrix.SetOrthoProjection(-3.55 * 2, 3.55 * 2, -2.0f * 2, 2.0f * 2, -1.0f * 2, 1.0f * 2);
	projectionMatrix.SetOrthoProjection(-3.55 * 100, 3.55 * 100, -2.0f * 100, 2.0f * 100, -1.0f * 100, 1.0f * 100);
	textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	textured.SetProjectionMatrix(projectionMatrix);
	textured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flare.Load("myMap.txt");
	spriteSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	viewMatrix.Translate(-flare.entities[0].x * 5, -flare.entities[0].y * 5, 0);
}


void ProcessEvents(float elapsed)
{
	if (keys[SDL_SCANCODE_UP]) { viewMatrix.Translate(0, -elapsed * 100, 0); }
	if (keys[SDL_SCANCODE_DOWN]) { viewMatrix.Translate(0, elapsed * 100, 0); }
	if (keys[SDL_SCANCODE_RIGHT]) { viewMatrix.Translate(-elapsed * 100, 0, 0); }
	if (keys[SDL_SCANCODE_LEFT]) { viewMatrix.Translate(elapsed * 100, 0, 0); }
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}


void Update(float& elapsed)
{

}

void Render()
{
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (flare.mapData[y][x] != 0) {
				float u = (float)(((int)flare.mapData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)flare.mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});
				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
					});

			}
		}
	}
	Matrix modelMatrix;
	textured.SetModelMatrix(modelMatrix);
	textured.SetViewMatrix(viewMatrix);
	glUseProgram(textured.programID);
	glBindTexture(GL_TEXTURE_2D, spriteSheet);
	glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(textured.positionAttribute);
	glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(textured.texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, (int)vertexData.size() / 2);
	glDisableVertexAttribArray(textured.positionAttribute);
	glDisableVertexAttribArray(textured.texCoordAttribute);
}


int main(int argc, char *argv[])
{
	Setup();
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClearColor(.529, .808, 1.222, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ProcessEvents(elapsed);
		Update(elapsed);
		Render();

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
