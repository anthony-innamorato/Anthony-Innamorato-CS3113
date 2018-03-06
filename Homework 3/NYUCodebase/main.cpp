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
#define MAX_BULLETS 5

struct Entity;
struct Bullet;
struct Number;

SDL_Window* displayWindow;
ShaderProgram textured;
ShaderProgram untextured;
Matrix projectionMatrix;
Matrix viewMatrix;
bool done = false;
SDL_Event event;
float lastFrameTicks = 0.0f;
std::vector<Entity*> entities;
std::vector<Entity*> enemyBullets;
std::vector<Entity*> playerBullets;
std::vector<Number*> numPtrs;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
bool startedGame = false;
bool endCase = false;
GLuint spriteSheet;
float speed = 4.0;
bool justShot = false;

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
	float rotation;
	GLuint textureImage;
};

struct EnemyBullet : public Entity
{
	EnemyBullet(int index) : Entity(spriteSheet, false), index(index) {}
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
		if (!endCase) {
			if (alive)
			{
				position.x -= elapsed * speed / 2;
				if (position.x <= -3.55 * 2)
				{
					alive = false;
					return;
				}
				bool collision = entities[0]->isColliding(position.x, position.y);
				if (collision)
				{
					endCase = true;
				}
			}
			else
			{
				for (int i = 0; i < 4; i++)
				{
					if (entities[index + i]->alive)
					{
						position = entities[index + i]->position;
						alive = true;
					}
				}
			}
		}
	}
	bool isColliding(float x, float y) const
	{
		return true;
	}

	float u = 0.0;
	float v = 3617.0 / 4096;
	float width = 422.0 / 4096.0;
	float height = 90.0 / 4096.0;
	float size = .5;
	int index;
};

struct PlayerBullet : public Entity
{
	PlayerBullet() : Entity(spriteSheet, false) { position.x = -3.55 * 2 + 3.5; }
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
				position.x = -3.55 * 2 + 3.5;
				return;
			}
			for (size_t i = 1; i < entities.size(); i++)
			{
				bool collision = entities[i]->isColliding(position.x, position.y);
				if (collision)
				{
					entities[i]->alive = false;
					alive = false;
					position.x = -3.55 * 2 + 3.5;
					justShot = true;
					return;
				}
			}
		}
	}
	bool isColliding(float x, float y) const
	{
		return true;
	}

	float u = 0.0;
	float v = 3709.0 / 4096;
	float width = 422.0 / 4096;
	float height = 90.0 / 4096;
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

	void Update(float elapsed)
	{
		if (!endCase)
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
				if (playerBullets[0]->alive == false && !justShot)
				{
					playerBullets[0]->position.y = position.y;
					playerBullets[0]->alive = true;
				}
			}
			else { justShot = false; }
		}
	}
	bool isColliding(float x, float y) const
	{
		return false;
	}

	float u = 0.0;
	float v = 0.0;
	float width = 3288.0 / 4096;
	float height = 888.0 / 4096;
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
		position.x -= elapsed * speed/20;
		if (position.x <= -3.55 * 2 + 5.5 && alive)
		{
			speed = 0.0;
			endCase = true;
		}
	}
	bool isColliding(float x, float y) const
	{
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
	void Draw(ShaderProgram* program, float offsetX, float offsetY)
	{
		modelMatrix.Translate(offsetX, offsetY, 0);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width/height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size };

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

struct TitleScreen
{
	TitleScreen() : texture(spriteSheet){}
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
	}
	GLuint texture;
	Matrix modelMatrix;
	float u = 0.0;
	float v = 890.0 / 4096;
	float width = 1870.0 / 4096;
	float height = 109.0/4096;
	float size = .5;
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
	untextured.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	untextured.SetProjectionMatrix(projectionMatrix);
	untextured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	spriteSheet = LoadTexture(RESOURCE_FOLDER"sprites.png");

	std::vector<std::vector<float>> coordVec;


}

//phantom
struct Enemy1 : public Enemy
{
	Enemy1(float y_pos) : Enemy(spriteSheet, 0, 0.0 / 4096.0, 3084.0 / 4096, 932.0 / 4096.0, 531.0 / 4096, .5) { position.x = 2.0; position.y = y_pos; }
};

//seraph
struct Enemy2 : public Enemy
{
	Enemy2(float y_pos) : Enemy(spriteSheet, 1, 0.0, 2013.0 / 4096, 1240.0 / 4096.0, 477.0 / 4096, .3) { position.x = 4.0; position.y = y_pos; }
};

//destroyer
struct Enemy3 : public Enemy
{
	Enemy3(float y_pos) : Enemy(spriteSheet, 2, 0.0, 2492.0 / 4096, 1027.0 / 4096.0, 590.0 / 4096, .5) { position.x = 6.0; position.y = y_pos; }
};

//curiser
struct Enemy4 : public Enemy
{
	Enemy4(float y_pos) : Enemy(spriteSheet, 3, 0.0, 1001.0 / 4096, 1869.0 / 4096.0, 1010.0 / 4096, .5) { position.x = 8.0; position.y = y_pos; }
};

void ProcessEvents()
{
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
			{
				startedGame = true;
			}
		}
	}
	//our SDL event loop
	//check input events
}

void drawScore(ShaderProgram* program)
{
	float offsetX = 0.0;
	float offsetY = -3.8;
	if (endCase) { offsetY = -.7; }
	
	//i tried making this simpler with globals but i was using them wrong, so ended up using this approach instead :(((
	int sum = 0;
	bool first = false;
	for (Entity* ptr : entities)
	{
		if (!ptr->alive && first)
		{
			if (sum == 0)
			{
				sum = 2;
			}
			else
			{
				sum *= 2;
			}
		}
		else
		{
			first = true;
		}
	}
	std::string sumStr = std::to_string(sum);
	for (char c : sumStr)
	{
		int res = c - '0';
		numPtrs[res]->Draw(program, offsetX, offsetY);
		offsetX += .5;
	}
}

void drawBound(ShaderProgram* program)
{
	Matrix modelMatrix;
	modelMatrix.Translate(-3.55 * 2 + 4.6, 0.0, 0.0f);

	untextured.SetProjectionMatrix(projectionMatrix);
	untextured.SetViewMatrix(viewMatrix);
	untextured.SetModelMatrix(modelMatrix);

	float vertices[] = { 0.0, -1000.0, 0.0f, 1000.0, -.1, 1000.0 };
	glVertexAttribPointer(untextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(untextured.positionAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(untextured.positionAttribute);
}

void Update(float& elapsed)
{
	int alive = 0;
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Update(elapsed);
		if (entities[i]->alive) { alive++; }
	}
	if (alive == 1) { endCase = true; }
	for (size_t i = 0; i < playerBullets.size(); i++)
	{
		playerBullets[i]->Update(elapsed);
	}
	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		enemyBullets[i]->Update(elapsed);
	}
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
	drawScore(&textured);
	drawBound(&untextured);
}

void runTitleScreen(TitleScreen* t)
{
	t->Draw(&textured);
}


int main(int argc, char *argv[])
{
	Setup();
	Player* p1 = new Player();
	entities.push_back(p1);

	for (size_t i = 0; i < 5; i++)
	{
		EnemyBullet* eb = new EnemyBullet(i*4 + 1);
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

	//
	for (int i = 0; i < 10; i++)
	{
		//int index, float u, float v, float width, float height, float size
		if (i == 0)
		{
			Number* num = new Number(0, 186.0 / 4096, 3919.0 / 4096, 89.0 / 4096, 101.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 1)
		{
			Number* num = new Number(1, 368.0 / 4096, 3801.0 / 4096, 69.0 / 4098, 101.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 2)
		{
			Number* num = new Number(2, 367.0 / 4096, 3904.0 / 4096, 87.0 / 4096, 102.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 3)
		{
			Number* num = new Number(3, 94.0 / 4096, 3904.0 / 4096, 90.0 / 4098, 102.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 4)
		{
			Number* num = new Number(4, 100.0 / 4096, 3801.0 / 4096, 80.0 / 4098, 99.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 5)
		{
			Number* num = new Number(5, 0.0, 3904.0 / 4096, 92.0 / 4098, 101.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 6)
		{
			Number* num = new Number(6, 186.0 / 4096, 3801.0 / 4096, 89.0 / 4098, 116.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 7)
		{
			Number* num = new Number(7, 0.0, 3801.0 / 4096, 98.0 / 4098, 101.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else if (i == 8)
		{
			Number* num = new Number(8, 277.0 / 4096, 3801.0 / 4096, 89.0 / 4098, 101.0 / 4096, .5);
			numPtrs.push_back(num);
		}
		else
		{
			Number* num = new Number(9, 277.0 / 4096, 3904.0 / 4096, 88.0 / 4098, 102.0 / 4096, .5);
			numPtrs.push_back(num);
		}
	}

	TitleScreen* t = new TitleScreen();


	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		while (!startedGame)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			runTitleScreen(t);
			ProcessEvents();
			SDL_GL_SwapWindow(displayWindow);
		}

		glClear(GL_COLOR_BUFFER_BIT);


		ProcessEvents();
		Update(elapsed);
		Render();


		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
