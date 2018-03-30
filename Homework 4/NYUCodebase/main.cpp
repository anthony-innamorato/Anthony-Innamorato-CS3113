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
#include "FlareMap.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

class Entity;
class Player;
class Enemy;

SDL_Window* displayWindow;
ShaderProgram textured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
SDL_Event event;
float lastFrameTicks = 0.0f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
float TILE_SIZE = 16.0;
int LEVEL_HEIGHT = 16;
int LEVEL_WIDTH = 32;
int SPRITE_COUNT_X = 16;
int SPRITE_COUNT_Y = 8;
GLuint spriteSheet;
vector<Entity*> entities;

void tileMapCollision(Entity& entity);
struct Vector
{
	Vector() {}
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;
};

struct TextCoords 
{
	TextCoords(float left = 0, float right = 1, float top = 0, float bot = 1)
		: left(left), right(right), top(top), bot(bot) {}
	float left;
	float right;
	float bot;
	float top;
};

void worldToTileCoordinates(float worldX, float worldY, int* gridX, int* gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

struct Entity
{
	Entity() {}
	Entity(const GLuint& texture, bool alive) : textureImage(texture), alive(alive) {}
	virtual void Draw(ShaderProgram* program) = 0;
	virtual void Update(float elapsed) = 0;
	virtual bool isColliding(float x, float y) const = 0;
	bool alive = true;
	Matrix modelMatrix;
	Vector position;
	float rotation;
	GLuint textureImage;
	Vector gravity;
	Vector velocity;
	Vector halfLengths;
	TextCoords textCoords = (0.0, 1.0, 0.0, 1.0);
	Vector friction;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
};

struct Player : public Entity
{
	Player() : Entity(spriteSheet, true) {}
	void Draw(ShaderProgram* program)
	{
		if (!alive) { return; }
		modelMatrix.Identity();
		modelMatrix.Translate(position.x, position.y, 0);
		glUseProgram(textured.programID);
		textured.SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, textureImage);

		float vertices[] =
		{
			-halfLengths.x, -halfLengths.y, halfLengths.x, -halfLengths.y, halfLengths.x, halfLengths.y,
			-halfLengths.x, -halfLengths.y, halfLengths.x, halfLengths.y, -halfLengths.x, halfLengths.y
		};

		glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(textured.positionAttribute);

		GLfloat texCoords[] =
		{
			textCoords.left, textCoords.bot,
			textCoords.right, textCoords.bot,
			textCoords.right, textCoords.top,
			textCoords.left, textCoords.bot,
			textCoords.right, textCoords.top,
			textCoords.left, textCoords.top 
		};

		glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(textured.positionAttribute);
		glDisableVertexAttribArray(textured.texCoordAttribute);
	}

	void Update(float elapsed)
	{
		tileMapCollision(*this);
		if (velocity.x >= .05) { velocity.x -= friction.x * elapsed; }
		else if (velocity.x <= -.05) { velocity.x += friction.x * elapsed; }
		velocity.y += gravity.y * elapsed;

		position.x += velocity.x *  elapsed;
		position.y += velocity.y *  elapsed;

	}
	bool isColliding(float x, float y) const
	{
		/*
		if (x <= position.x + 2.3 && y <= position.y + .4 && y >= position.y - .35) { return true; }
		return false;
		*/
		return true;
	}
};

struct Enemy : public Entity
{
	Enemy() {}
	void Draw(ShaderProgram* program)
	{
		if (!alive) { return; }
		modelMatrix.Identity();
		modelMatrix.Translate(position.x, position.y, 0);
		glUseProgram(textured.programID);
		textured.SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, textureImage);

		float vertices[] =
		{
			-halfLengths.x, -halfLengths.y, halfLengths.x, -halfLengths.y, halfLengths.x, halfLengths.y,
			-halfLengths.x, -halfLengths.y, halfLengths.x, halfLengths.y, -halfLengths.x, halfLengths.y
		};

		glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(textured.positionAttribute);

		GLfloat texCoords[] =
		{
			textCoords.left, textCoords.bot,
			textCoords.right, textCoords.bot,
			textCoords.right, textCoords.top,
			textCoords.left, textCoords.bot,
			textCoords.right, textCoords.top,
			textCoords.left, textCoords.top
		};

		glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(textured.positionAttribute);
		glDisableVertexAttribArray(textured.texCoordAttribute);
	}
	void Update(float elapsed)
	{
		tileMapCollision(*this);
		velocity.y += gravity.y * elapsed;

		position.x += velocity.x *  elapsed;
		position.y += velocity.y *  elapsed;
	}
	bool isColliding(float x, float y) const
	{
		/*
		if (!alive)
		{
			return false;
		}
		if (x >= position.x - 1.8 && y <= position.y + .25 && y >= position.y - .25)
		{
			return true;
		}
		else
		{
			return false;
		}
		*/
		return true;
	}
};


Player p1;
Enemy enemy;

void placeEntity(const string& type, float x, float y) 
{
	if (type == "Player")
	{
		p1.position = Vector(x * TILE_SIZE, -2.5 * 2 * TILE_SIZE, 0);
		p1.halfLengths = Vector(TILE_SIZE / 2, TILE_SIZE / 2, 0);
		p1.gravity = Vector(0, -100, 0);
		p1.friction = Vector(100, 0, 0);
		entities.push_back(&p1);
	}
	if (type == "Enemy")
	{
		enemy.position = Vector(x * TILE_SIZE, -2.5 * 2 * TILE_SIZE, 0);
		enemy.halfLengths = Vector(TILE_SIZE / 2, TILE_SIZE / 2, 0);
		enemy.gravity = Vector(0, -100, 0);
		enemy.friction = Vector(100, 0, 0);
		entities.push_back(&enemy);
	}
}

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


FlareMap flare;

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
	projectionMatrix.SetOrthoProjection(-3.55 * 50, 3.55 * 50, -2.0f * 50, 2.0f * 50, -1.0f * 50, 1.0f * 50);
	textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	textured.SetProjectionMatrix(projectionMatrix);
	textured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flare.Load("myMap.txt");
	if (flare.flareEntities.size() == 0) { done = true; }
	for (size_t i = 0; i < flare.flareEntities.size(); i++)
	{
		FlareMapEntity curr = flare.flareEntities[i];
		placeEntity(curr.type, curr.x, curr.y);
	}
	spriteSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	//viewMatrix.Translate(-flare.entities[0].y * 20, -flare.entities[0].x * 20, 0);
}


void ProcessEvents(float elapsed)
{
	if (keys[SDL_SCANCODE_RIGHT]) { p1.velocity.x += .5; }
	if (keys[SDL_SCANCODE_LEFT]) { p1.velocity.x -= .5; }
	if (keys[SDL_SCANCODE_SPACE] && p1.collidedBottom) { p1.velocity.y += 75; p1.collidedBottom = false; }
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}


void Update(float elapsed)
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Update(elapsed);
	}
	viewMatrix.Identity();
	viewMatrix.Translate(-p1.position.x, -p1.position.y, 0);
	textured.SetViewMatrix(viewMatrix);
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

	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Draw(&textured);
	}
}

void tileMapCollision(Entity& entity)
{
	int top, bottom, left, right, x, y, garb;
	worldToTileCoordinates(entity.position.x + entity.halfLengths.x, entity.position.y, &right, &garb);
	worldToTileCoordinates(entity.position.x - entity.halfLengths.x, entity.position.y, &left, &garb);
	worldToTileCoordinates(entity.position.x, entity.position.y + entity.halfLengths.y, &garb, &top);
	worldToTileCoordinates(entity.position.x, entity.position.y - entity.halfLengths.y, &garb, &bottom);
	worldToTileCoordinates(entity.position.x, entity.position.y, &x, &y);

	float penetration;

	if (bottom <= 1 || top <= 1 || top >= LEVEL_HEIGHT - 1 || bottom >= LEVEL_HEIGHT - 1)
	{
		entity.velocity.y = 0;
		entity.position.y -= .001;
	}

	if (left <= 1 || right <= 1 || left >= LEVEL_WIDTH - 1 || right >= LEVEL_WIDTH - 1)
	{
		entity.velocity.x = 0;
		entity.position.x -= .001;
	}

	if (flare.mapData[bottom][x] != 0)
	{
		penetration = fabs(fabs(-TILE_SIZE * bottom) - fabs(entity.position.y - entity.halfLengths.y));
		entity.position.y += penetration + .001;
		entity.velocity.y = 0;
		entity.collidedBottom = true;
	}

	if (flare.mapData[top][x] != 0)
	{
		penetration = fabs(fabs(entity.position.y + entity.halfLengths.y) - fabs(((-TILE_SIZE * top) - TILE_SIZE)));
		entity.position.y -= penetration + .001;
		entity.velocity.y = 0;
		entity.collidedTop = true;
	}
	if (flare.mapData[y][left] != 0)
	{
		penetration = fabs(fabs(entity.position.x - entity.halfLengths.x) - fabs(((TILE_SIZE*left) + TILE_SIZE)));
		entity.position.x += penetration + .001;
		entity.velocity.x = 0;
		entity.collidedLeft = true;
	}
	if (flare.mapData[y][right] != 0)
	{
		penetration = fabs(fabs(TILE_SIZE * right) - fabs(entity.position.x + entity.halfLengths.x));
		entity.position.x -= penetration + .001;
		entity.velocity.x = 0;
		entity.collidedRight = true;
	}
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
