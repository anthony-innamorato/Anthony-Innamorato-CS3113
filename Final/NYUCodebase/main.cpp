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
#include <math.h>
#include <cmath>
using namespace std;

SDL_Window* displayWindow;
ShaderProgram textured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
int counter = 0;
SDL_Event event;
float lastFrameTicks = 0.0f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

struct Entity;
std::vector<Entity*> entities;
vector<Entity*> starsVec;
Entity* playerBullet;
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
		: textureImage(texture), u(u / 4096.0), v(v / 4096.0), width(width / 4096.0), height(height / 4096.0), size(size), position(position), 
			xScale(xScale), yScale(yScale), angle(angle)
	{

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
			modelMatrix.Identity();
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
	virtual void shoot(Entity* owner) {}
	bool alive = false;
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
	bool isEnemy = false;
};

float distance(Entity* e1, Entity* e2)
{
	return abs(sqrt(pow(e2->position.x - e1->position.x, 2) + pow(e2->position.y - e1->position.y, 2)));
}

bool collisions(Entity* entity1, Entity* entity2)
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
		if (entity1 == entities[0])
		{
			entity1->position.x += (penetration.first * 0.5f);
			entity1->position.y += (penetration.second * 0.5f);
		}
		else if (entity2 == entities[0])
		{
			entity2->position.x -= (penetration.first * 0.5f);
			entity2->position.y -= (penetration.second * 0.5f);
		}
	}
	return collided;
}

struct Bullet : public Entity
{
	Bullet(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {
		alive = false;
	}

	void shoot(Entity* owner)
	{
		position = entities[0]->position;
		alive = true;
		angle = owner->angle;
	}
	void update(float elapsed)
	{
		if (alive)
		{
			position.x += elapsed * -sin(angle * (3.14159265 / 180.0)) * 10;
			position.y += elapsed * cos(angle * (3.14159265 / 180.0)) * 10;
			timeAlive += elapsed;
			//distance to player and run collisions on enemies
			if (timeAlive > .5)
			{
				alive = false;
				timeAlive = 0;
			}
		}
	}
	float timeAlive = 0.0;
};

struct Star : public Entity
{
	Star(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {
		alive = true;
	}
	void update(float elapsed)
	{
		if (alive)
		{
			position.x -= elapsed;
			if (position.x <= -17.75)
			{
				position.x = 17.75;
				return;
			}
		}
	}

};

struct Player : public Entity
{
	Player(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle) 
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct AI : public Entity
{
	AI(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {}
};

struct Enemy : public Entity
{
	Enemy(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle)
		: Entity(texture, u, v, width, height, size, position, xScale, yScale, angle) {
		isEnemy = true;
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
	projectionMatrix.SetOrthoProjection(-3.55 * 3, 3.55 * 3, -2.0f * 3, 2.0f * 3, -1.0f * 3, 1.0f * 3);
	textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	textured.SetProjectionMatrix(projectionMatrix);
	textured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	spriteSheet = LoadTexture(RESOURCE_FOLDER"sprites.png");


	Vector p1Vec = Vector(-10.0, 0.0, 0.0);
	Vector e1Vec = Vector(0.0, 0.0, 0.0);
	Player* p1 = new Player(spriteSheet, 887.0, 3734.0, 241.0, 159.0, 1.0, p1Vec, .75, .75, 0.0);
	p1->alive = true;
	entities.push_back(p1);
	//Enemy(const GLuint& texture, float u, float v, float width, float height, float size, Vector position, float xScale, float yScale, float angle)
	Enemy* e1 = new Enemy(spriteSheet, 887.0, 3570.0, 162.0, 173.0, 1.0, e1Vec, 10.0, 10.0, 0.0);
	e1->alive = true;
	entities.push_back(e1);
	//create one player bullet
	//height = "855" width = "1403" y = "2398" x = "0"
	playerBullet = new Bullet(spriteSheet, 0.0, 2398.0, 1403.0, 855.0, .05, p1Vec, 10.0, 10.0, 0.0);
	float y = 12.0;
	float x = -14.0;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			//height = "92" width = "90" y = "3992" x = "465" name = "star.png" / >
			Vector starVec = Vector(x, y, 0.0);
			Star* star = new Star(spriteSheet, 465.0, 3992.0, 90.0, 92.0, 1.0, starVec, .1, .1, 0.0);
			x += 3;
			starsVec.push_back(star);
		}
		y -= 3;
		x = -14.0;
	}
}

void ProcessEvents(float elapsed)
{
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
	if (keys[SDL_SCANCODE_W] && entities[0]->position.y + entities[0]->halfLengths.y <= 2.0 * 5)
	{
		entities[0]->position.y += elapsed * 5.0; 
	}
	if (keys[SDL_SCANCODE_A] && entities[0]->position.x - entities[0]->halfLengths.x >= -10.0)
	{
		entities[0]->position.x -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_S] && entities[0]->position.y - entities[0]->halfLengths.y >= -2.0 * 5)
	{
		entities[0]->position.y -= elapsed * 5.0;
	}
	if (keys[SDL_SCANCODE_D] && entities[0]->position.x + entities[0]->halfLengths.y <= 10.0)
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
	if (keys[SDL_SCANCODE_SPACE])
	{
		if (!playerBullet->alive)
		{
			playerBullet->shoot(entities[0]);
		}
	}
}

void Update(float elapsed)
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->update(elapsed);
		for (size_t j = i; j < entities.size(); j++)
		{
			if (i != j)
			{
				collisions(entities[i], entities[j]);
			}
		}
	}
	viewMatrix.Identity();
	viewMatrix.Translate(-entities[0]->position.x, -entities[0]->position.y, 0);
	textured.SetViewMatrix(viewMatrix);
	for (Entity* star : starsVec)
	{
		star->update(elapsed);
	}
	playerBullet->update(elapsed);
	for (Entity* bullet : enemyBullets)
	{
		bullet->update(elapsed);
	}
}

void Render()
{
	for (Entity* curr : starsVec)
	{
		curr->draw();
	}
	for (Entity* curr : entities)
	{
		curr->draw();
	}
	if (playerBullet->alive) { playerBullet->draw(); }
	for (Entity* bullet : enemyBullets)
	{
		bullet->draw();
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
