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
#define MAX_BULLETS 5

struct Entity;
struct Bullet;

SDL_Window* displayWindow;
ShaderProgram textured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
SDL_Event event;
float lastFrameTicks = 0.0f;
std::vector<Entity*> entities;
std::vector<Entity*> enemyBullets;
std::vector<Entity*> playerBullets;
int score;
bool first;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

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
	Entity(const GLuint& texture, bool alive) : textureImage(texture), position(-3.55 * 2 + 1.2, 0, 0), alive(alive) {}
	virtual void Draw(ShaderProgram* program) = 0;
	virtual void Update(float elapsed) = 0;
	virtual bool isColliding(float x, float y) const = 0;
	bool alive;
	Matrix modelMatrix;
	Vector position;
	float speed = 4.0;
	Vector size;
	float rotation;
	GLuint textureImage;
};

struct EnemyBullet : public Entity
{
	EnemyBullet() : Entity(spriteSheet, false) {}
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
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
	void Update(float elapsed)
	{

	}
	bool isColliding(float x, float y) const
	{
		return true;
	}
	float u = 0.0;
	float v = 1902.0 / 2048;
	float width = 422.0 / 4096.0;
	float height = 90.0 / 2048;
	float size = 1.0;
};

struct PlayerBullet : public Entity
{
	PlayerBullet() : Entity(spriteSheet, false) { position.x = -4.2; }
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
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
	void Update(float elapsed)
	{
		if (alive) 
		{
			position.x += elapsed * speed * 2;
			if (position.x >= 7.1)
			{
				alive = false;
				position.x = -4.2;
				return;
			}
			for (size_t i = 1; i < entities.size(); i++)
			{
				bool collision = entities[i]->isColliding(position.x, position.y);
				if (collision)
				{
					entities[i]->alive = false;
					alive = false;
					position.x = -5;
					if (first) { score = 2; first = false; }
					else { score *= 2; }
				}
			}
		}
	}
	bool isColliding(float x, float y) const
	{
		return true;
	}

	float u = 424.0 / 4096;
	float v = 1902.0 / 2048;
	float width = 422.0 / 4096;
	float height = 90.0 / 2048;
	float size = .5;
};

struct Player : public Entity
{
	Player() : Entity(spriteSheet, true) {}
	void Draw(ShaderProgram* program)
	{
		modelMatrix.Translate(position.x, position.y, 0);
		glUseProgram(textured.programID);
		textured.SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, textureImage);

		float aspect = (width*4096) / (height*2048);
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

	void Update(float elapsed)
	{
		if (keys[SDL_SCANCODE_UP])
		{
			//move player right * elapsed
			if (position.y <= 1.8 * 2)
			{
				position.y += elapsed * speed;
			}
		}
		if (keys[SDL_SCANCODE_DOWN])
		{
			if (position.y >= -1.8 * 2)
			{
				position.y -= elapsed * speed;
			}
			//move player left * elapsed
		}
		if (keys[SDL_SCANCODE_SPACE])
		{
			if (playerBullets[0]->alive == false)
			{
				playerBullets[0]->position.y = position.y;
				playerBullets[0]->alive = true;
			}
		}
	}
	bool isColliding(float x, float y) const
	{
		return true;
	}

	float u = 0.0;
	float v = 0.0;
	float width = 3288.0 / 4096;
	float height = 888.0 / 2048;
	float size = .7;
};

struct Enemy : public Entity
{
	Enemy(const GLuint& texture, int bulletIndex, float u, float v, float width, float height, float size) 
		: Entity(texture, true) , bullet(enemyBullets[bulletIndex]), u(u), v(v), width(width), height(height), size(size){}
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
			glUseProgram(textured.programID);
			textured.SetModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, textureImage);
			float aspect = (width * 4096) / (height * 2048);
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
	void Update(float elapsed)
	{
		position.x -= elapsed * speed/50;
		if (position.x <= -3.55 * 2 + 2.5)
		{
			done = true;
		}
	}
	bool isColliding(float x, float y) const
	{
		if (!alive)
		{
			return false;
		}
		if (x >= position.x - 1 && y <= position.y + .3 && y >= position.y - .3)
		{
			return true;
		}
		else
		{
			return false;
		}

	}
	Entity* bullet;
	float u;
	float v;
	float width;
	float height;
	float size;
};


struct Number
{
	Number(int index, float u, float v, float width, float height, float size) : texture(spriteSheet), index(index), u(u), v(v), width(width), height(height), size(size){}
	void Draw(ShaderProgram* program)
	{
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size };

		//modelMatrix.Translate(position.x, position.y, 0);
		glUseProgram(textured.programID);
		textured.SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(textured.positionAttribute);

		glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(textured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(textured.positionAttribute);
		glDisableVertexAttribArray(textured.texCoordAttribute);
		modelMatrix.Identity();
	}
	int index;
	GLuint texture;
	Matrix modelMatrix;
	float u;
	float v;
	float width;
	float height;
	float size;
};

void drawNum(int num, ShaderProgram* program)
{
	//Number instance = Number(num);
	//instance.Draw(program);
}

void drawScore(ShaderProgram* program)
{
	int curr = 0;
	while (score << curr)
	{
		drawNum(curr, program);
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

//phantom
struct Enemy1 : public Enemy
{
	Enemy1(float y_pos) : Enemy(spriteSheet, 0, 2900.0 / 4096.0, 1369.0 / 2048, 932.0 / 4096.0, 531.0 / 2048, .7) { position.x = 0.0; position.y = y_pos; }
};

//seraph
struct Enemy2 : public Enemy
{
	Enemy2(float y_pos) : Enemy(spriteSheet, 1, 1871.0 / 4096.0, 890.0 / 2048, 1240.0 / 4096.0, 477.0 / 2048, .7) { position.x = 2.0; position.y = y_pos; }
};

//destroyer
struct Enemy3 : public Enemy
{
	Enemy3(float y_pos) : Enemy(spriteSheet, 2, 1871.0 / 4096.0, 1369.0 / 2048, 1027.0 / 4096.0, 590.0 / 2048, .7) { position.x = 4.0; position.y = y_pos; }
};

//curiser
struct Enemy4 : public Enemy
{
	Enemy4(float y_pos) : Enemy(spriteSheet, 3, 0.0 / 4096.0, 890.0 / 2048, 1869.0 / 4096.0, 1010.0 / 2048, .7) { position.x = 6.0; position.y = y_pos; }
};

void ProcessEvents()
{
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
	//our SDL event loop
	//check input events
}

void Update(float& elapsed)
{
	int alive = 0;
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Update(elapsed);
		if (entities[i]->alive) { alive++; }
	}
	if (alive == 1) { done = true; }
	for (size_t i = 0; i < playerBullets.size(); i++)
	{
		playerBullets[i]->Update(elapsed);
	}
	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		enemyBullets[i]->Update(elapsed);
	}
	drawScore(&textured);
	//move stuff and check for collisions
}

void Render()
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Draw(&textured);
	}
	for (size_t i = 0; i < playerBullets.size(); i++)
	{
		playerBullets[i]->Draw(&textured);
	}
	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		enemyBullets[i]->Draw(&textured);
	}
	//for all game elements
	//setup transdorms, render sprites
}


int main(int argc, char *argv[])
{
	Setup();
	Player* p1 = new Player();
	entities.push_back(p1);

	for (size_t i = 0; i < 4; i++)
	{
		EnemyBullet* eb = new EnemyBullet();
		PlayerBullet* pb = new PlayerBullet();
		enemyBullets.push_back(eb);
		playerBullets.push_back(pb);
	}

	std::vector<Enemy1*> en1Vec;
	std::vector<Enemy2*> en2Vec;
	std::vector<Enemy3*> en3Vec;
	std::vector<Enemy4*> en4Vec;
	float init_y = 3.1;
	for (size_t i = 0; i < 5; i++)
	{
		Enemy1* phant = new Enemy1(init_y);
		Enemy2* ser = new Enemy2(init_y);
		Enemy3* dest = new Enemy3(init_y);
		Enemy4* cruis = new Enemy4(init_y);
		en1Vec.push_back(phant);
		en2Vec.push_back(ser);
		en3Vec.push_back(dest);
		en4Vec.push_back(cruis);
		init_y -= 1.5;
	}
	for (Enemy1* e1: en1Vec)
	{
		entities.push_back(e1);
	}
	for (Enemy2* e2 : en2Vec)
	{
		entities.push_back(e2);
	}
	for (Enemy3* e3 : en3Vec)
	{
		entities.push_back(e3);
	}
	for (Enemy4* e4 : en4Vec)
	{
		entities.push_back(e4);
	}


	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);

		ProcessEvents();
		Update(elapsed);
		Render();

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
