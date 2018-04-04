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
#include <vector>
#include <string>

SDL_Window* displayWindow;
ShaderProgram textured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
SDL_Event event;
float lastFrameTicks = 0.0f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

struct Entity;
std::vector<Entity*> entities;
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

struct Entity
{
	Entity() {}
	Entity(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle) 
		: textureImage(texture), u(u / 256.0), v(v / 256.0), width(width / 256.0), height(height / 256.0), size(size), position(position), 
			xScale(xScale), yScale(yScale), angle(angle)
	{
		modelMatrix.Translate(position.x, position.y, position.z);
		modelMatrix.Scale(xScale, yScale, 1.0);
		modelMatrix.Rotate(angle * (3.14159265 / 180.0));
	}

	void draw()
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, position.z);
			modelMatrix.Scale(xScale, yScale, 1.0);
			modelMatrix.Rotate(angle * (3.14159265 / 180.0));
			glUseProgram(textured.programID);
			textured.SetModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, textureImage);
			float aspect = width / height;
			float vertices[] = {
				-0.5f * size * aspect, -0.5f * size,
				0.5f * size * aspect, 0.5f * size,
				-0.5f * size * aspect, 0.5f * size,
				0.5f * size * aspect, 0.5f * size,
				-0.5f * size * aspect, -0.5f * size ,
				0.5f * size * aspect, -0.5f * size };
			glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(textured.positionAttribute);

			GLfloat texCoords[] = {
				u, v + height,
				u + width, v,
				u, v,
				u + width, v,
				u, v + height,
				u + width, v + height
			};
			glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(textured.texCoordAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(textured.positionAttribute);
			glDisableVertexAttribArray(textured.texCoordAttribute);
			modelMatrix.Identity();
		}
	}

	virtual void update(float elapsed) {}
	bool alive = true;
	Matrix modelMatrix;
	Vector position;
	GLuint textureImage;
	float u;
	float v;
	float width;
	float height;
	float size;
	float xScale;
	float yScale;
	float angle;
};

struct Player : public Entity
{
	Player(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle) 
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct Enemy : public Entity
{
	Enemy(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle, bool moveLeft, bool moveUp)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle), moveLeft(moveLeft), moveUp(moveUp) {}

	void update(float elapsed)
	{
		if (moveLeft) { position.x -= elapsed * 1.0; }
		else { position.x += elapsed * 2.0; }
		if (moveUp) { position.y += elapsed * 2.0; }
		else { position.y -= elapsed * 1.0; }
		if (position.x >= 3.55 * 2 + 2.0) { position.x *= -1.0; }
		else if (position.x <= -3.55 * 2 - 2.0) { position.x *= -1.0; }
		if (position.y <= -2.0 * 2 -2.0 ) { position.y *= -1.0; }
		else if (position.y >= 2.0 * 2 + 2.0) { position.y *= -1.0; }
	}
	bool moveLeft;
	bool moveUp;
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
	projectionMatrix.SetOrthoProjection(-3.55 * 2, 3.55 * 2, -2.0f * 2, 2.0f * 2, -1.0f * 2, 1.0f * 2);
	textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	textured.SetProjectionMatrix(projectionMatrix);
	textured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	spriteSheet = LoadTexture(RESOURCE_FOLDER"sprites.png");
	Vector p1Vec = Vector(0.0, 0.0, 0.0);
	Vector e1Vec = Vector(-2.0, 2.0, 0.0);
	Vector e2Vec = Vector(2.0, 2.0, 0.0);
	Player* p1 = new Player(spriteSheet, 0.0, 0.0, 159.0, 168.0, 1.0, p1Vec, .75, .75, 0.0);
	entities.push_back(p1);
	Enemy* e1 = new Enemy(spriteSheet, 0.0, 170.0, 47.0, 48.0, 1.0, e1Vec, 1.5, 1.5, 45.0, true, true);
	entities.push_back(e1);
	Enemy* e2 = new Enemy(spriteSheet, 49.0, 170.0, 44.0, 44.0, 1.0, e2Vec, 2.0, 2.0, 95.0, false, false);
	entities.push_back(e2);
}

void ProcessEvents(float elapsed)
{
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
	if (keys[SDL_SCANCODE_W])
	{
		entities[0]->position.y += elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_A])
	{
		entities[0]->position.x -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_S])
	{
		entities[0]->position.y -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_D])
	{
		entities[0]->position.x += elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_LEFT])
	{
		entities[0]->angle += elapsed * 100.0;
	}
	if (keys[SDL_SCANCODE_RIGHT])
	{
		entities[0]->angle -= elapsed * 100.0;
	}
}

void Update(float elapsed)
{
	for (Entity* curr : entities)
	{
		curr->update(elapsed);
	}
}

void Render()
{
	for (Entity* curr : entities)
	{
		curr->draw();
	}
}

int main(int argc, char *argv[])
{
	Setup();
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);


		ProcessEvents(elapsed);
		Update(elapsed);
		Render();
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
