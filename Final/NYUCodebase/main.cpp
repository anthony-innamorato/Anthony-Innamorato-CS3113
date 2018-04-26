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
#include "SatCollision.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <string>
using namespace std;

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
vector<Entity*> starsVec;
vector<Entity*> playerBullets;
vector<Entity*> enemyBullets;
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
	Vector operator*(Matrix& matrix) {
		return Vector(matrix.m[0][0] * x + matrix.m[1][0] * y + matrix.m[2][0] * z + matrix.m[3][0],
			matrix.m[0][1] * x + matrix.m[1][1] * y + matrix.m[2][1] * z + matrix.m[3][1],
			matrix.m[0][2] * x + matrix.m[1][2] * y + matrix.m[2][2] * z + matrix.m[3][2]);
	}
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
		halfLengths = Vector(size * (width / height) * .5, size * .5, 0);
		points.push_back(Vector(-halfLengths.x, -halfLengths.y, 0) * modelMatrix);
		points.push_back(Vector(halfLengths.x, -halfLengths.y, 0) * modelMatrix);
		points.push_back(Vector(halfLengths.x, halfLengths.y, 0) * modelMatrix);
		points.push_back(Vector(-halfLengths.x, halfLengths.y, 0) * modelMatrix);
	}

	virtual void draw()
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

			halfLengths = Vector(size * (width / height) * .5, size * .5, 0);
			points.clear();
			points.push_back(Vector(-halfLengths.x, -halfLengths.y, 0) * modelMatrix);
			points.push_back(Vector(halfLengths.x, -halfLengths.y, 0) * modelMatrix);
			points.push_back(Vector(halfLengths.x, halfLengths.y, 0) * modelMatrix);
			points.push_back(Vector(-halfLengths.x, halfLengths.y, 0) * modelMatrix);

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
	std::vector<Vector> points;
	Vector halfLengths;
	float xSpeed = 2.0;
	float ySpeed = 4.0;
	bool isEnemy = false;
};

struct Bullet : public Entity
{
	Bullet(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle, bool moveLeft, bool moveUp)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct Star : public Entity
{
	Star(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle, bool moveLeft, bool moveUp)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct Player : public Entity
{
	Player(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle) 
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct AI : public Entity
{
	AI(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle, bool moveLeft, bool moveUp)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct Enemy : public Entity
{
	Enemy(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle, bool moveLeft, bool moveUp)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {
		isEnemy = true;
	}

	void update(float elapsed)
	{
		return;
	}
	void draw()
	{
		for (Entity* curr : AIvec)
		{
			curr->draw();
		}
		Entity::draw();
	}
	std::vector<Entity*> AIvec;
};

void collisions(Entity* entity1, Entity* entity2)
{
	std::pair<float, float> penetration;

	std::vector<std::pair<float, float>> e1Points;
	std::vector<std::pair<float, float>> e2Points;

	for (int i = 0; i < entity1->points.size(); i++) {
		Vector point = entity1->points[i] * entity1->modelMatrix;
		e1Points.push_back(std::make_pair(point.x, point.y));
	}

	for (int i = 0; i < entity2->points.size(); i++) {
		Vector point = entity2->points[i] * entity2->modelMatrix;
		e2Points.push_back(std::make_pair(point.x, point.y));
	}

	bool collided = CheckSATCollision(e1Points, e2Points, penetration);
	if (collided)
	{
		entity1->position.x += (penetration.first * 0.5f);
		entity1->position.y += (penetration.second * 0.5f);

		entity2->position.x -= (penetration.first * 0.5f);
		entity2->position.y -= (penetration.second * 0.5f);
		if (entity1 != entities[0] && entity2 != entities[0])
		{
			entity1->xSpeed *= -1.1;
			entity1->ySpeed *= -1.1;
			entity2->xSpeed *= -1.1;
			entity2->ySpeed *= -1.1;
		}
		
		if (entity1 == entities[0] || entity2 == entities[0])
		{
			done = true;
		}
	}
}



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
}

void ProcessEvents(float elapsed)
{
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
	if (keys[SDL_SCANCODE_W] && entities[0]->position.y + entities[0]->halfLengths.y <= 2.0 * 2)
	{
		entities[0]->position.y += elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_A] && entities[0]->position.x - entities[0]->halfLengths.x >= -3.55 * 2)
	{
		entities[0]->position.x -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_S] && entities[0]->position.y - entities[0]->halfLengths.y >= -2.0 * 2)
	{
		entities[0]->position.y -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_D] && entities[0]->position.x + entities[0]->halfLengths.y <= 3.55 * 2)
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
/*
std::vector<Entity*> entities;
vector<Entity*> starsVec;
vector<Entity*> playerBullets;
vector<Entity*> enemyBullets;
vector<Entity*> AI;
*/

void Update(float elapsed)
{
	for (Entity* curr : entities)
	{
		curr->update(elapsed);
		for (Entity* curr2 : entities)
		{
			if (curr != curr2)
			{
				collisions(curr, curr2);
			}
		}
	}
}
/*
std::vector<Entity*> entities;
vector<Entity*> starsVec;
vector<Entity*> playerBullets;
vector<Entity*> enemyBullets;
*/
void Render()
{
	for (Entity* curr : entities)
	{
		curr->draw();
	}
	for (Entity* curr : starsVec)
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
