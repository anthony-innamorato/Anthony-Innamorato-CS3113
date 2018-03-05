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
//Bullet bullets[MAX_BULLETS];
const Uint8 *keys = SDL_GetKeyboardState(NULL);

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
	Entity(const GLuint& texture, bool alive) : textureImage(texture), position(-3.55 * 2 + .8, 0, 0), alive(alive) {}
	virtual void Draw(ShaderProgram* program) = 0;
	virtual void Update(float elapsed) = 0;
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
	EnemyBullet() : Entity(LoadTexture(RESOURCE_FOLDER"enemy_round.png"), false) {}
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
			glUseProgram(textured.programID);
			textured.SetModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, textureImage);
			float text_vertices[] = { -1.5 / 2, -0.5 / 2, 1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, 0.5 / 2 };
			//float text_vertices[] = { -1.5, -0.5, 1.5, -0.5, 1.5, 0.5, -1.5, -0.5, 1.5, 0.5, -1.5, 0.5 };
			glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
			glEnableVertexAttribArray(textured.positionAttribute);

			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
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
};

struct PlayerBullet : public Entity
{
	PlayerBullet() : Entity(LoadTexture(RESOURCE_FOLDER"player_round.png"), false) {}
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
			glUseProgram(textured.programID);
			textured.SetModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, textureImage);
			float text_vertices[] = { -1.5 / 2, -0.5 / 2, 1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, 0.5 / 2 };
			//float text_vertices[] = { -1.5, -0.5, 1.5, -0.5, 1.5, 0.5, -1.5, -0.5, 1.5, 0.5, -1.5, 0.5 };
			glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
			glEnableVertexAttribArray(textured.positionAttribute);

			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
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
		position.x += elapsed * speed ;
	}
};

struct Player : public Entity
{
	Player() : Entity(LoadTexture(RESOURCE_FOLDER"ship.png"), true) {}
	void Draw(ShaderProgram* program)
	{
		modelMatrix.Translate(position.x, position.y, 0);
		glUseProgram(textured.programID);
		textured.SetModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, textureImage);
		float text_vertices[] = { -1.5 / 2, -0.5 / 2, 1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, 0.5 / 2 };
		//float text_vertices[] = { -1.5, -0.5, 1.5, -0.5, 1.5, 0.5, -1.5, -0.5, 1.5, 0.5, -1.5, 0.5 };
		glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
		glEnableVertexAttribArray(textured.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
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
			playerBullets[0]->alive = true;
		}
	}

};

struct Enemy : public Entity
{
	Enemy(const GLuint& texture) : Entity(texture, true) {}
	void Draw(ShaderProgram* program)
	{
		if (alive)
		{
			modelMatrix.Translate(position.x, position.y, 0);
			glUseProgram(textured.programID);
			textured.SetModelMatrix(modelMatrix);

			glBindTexture(GL_TEXTURE_2D, textureImage);
			float text_vertices[] = { -1.5 / 2, -0.5 / 2, 1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, -0.5 / 2, 1.5 / 2, 0.5 / 2, -1.5 / 2, 0.5 / 2 };
			//float text_vertices[] = { -1.5, -0.5, 1.5, -0.5, 1.5, 0.5, -1.5, -0.5, 1.5, 0.5, -1.5, 0.5 };
			glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, text_vertices);
			glEnableVertexAttribArray(textured.positionAttribute);

			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
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
};

//phantom
struct Enemy1 : public Enemy
{
	Enemy1(float y_pos) : Enemy(LoadTexture(RESOURCE_FOLDER"phantom.png")) { position.x = 0.0; position.y = y_pos; }
};

//seraph
struct Enemy2 : public Enemy
{
	Enemy2(float y_pos) : Enemy(LoadTexture(RESOURCE_FOLDER"seraph.png")) { position.x = 2.0; position.y = y_pos; }
};

//destroyer
struct Enemy3 : public Enemy
{
	Enemy3(float y_pos) : Enemy(LoadTexture(RESOURCE_FOLDER"destroyer.png")) { position.x = 4.0; position.y = y_pos; }
};

//curiser
struct Enemy4 : public Enemy
{
	Enemy4(float y_pos) : Enemy(LoadTexture(RESOURCE_FOLDER"cruiser.jpg")) { position.x = 6.0; position.y = y_pos; }
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
}

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
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->Update(elapsed);
	}
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
	//for all game elements
	//setup transdorms, render sprites
}


int main(int argc, char *argv[])
{
	Setup();
	Player* p1 = new Player();
	entities.push_back(p1);

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

	for (size_t i = 0; i < 10; i++)
	{
		EnemyBullet* eb = new EnemyBullet();
		PlayerBullet* pb = new PlayerBullet();
		enemyBullets.push_back(eb);
		playerBullets.push_back(pb);
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
