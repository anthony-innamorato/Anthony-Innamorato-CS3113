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

/*
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
				position.x -= elapsed * speed * speedMult;
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
				int temp = index;
				for (int i = 0; i < 4; i++)
				{
					if (entities[temp]->alive)
					{
						position = entities[temp]->position;
						position.x -= 2;
						alive = true;
						if (i == 0) { speedMult = .3; }
						else if (i == 1) { speedMult = .8; }
						else if (i == 2) { speedMult = 1.5; }
						else { speedMult = 2.5; }
						return;
					}
					else { temp++; }
				}
			}
		}
	}
	bool isColliding(float x, float y) const
	{

		return false;
	}

	float speedMult;
	float u = 0.0;
	float v = 3617.0 / 4096;
	float width = 422.0 / 4096.0;
	float height = 90.0 / 4096.0;
	float size = .5;
	int index;
};
*/

/*
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
	}
	bool isColliding(float x, float y) const
	{
		if (x <= position.x + 2.3 && y <= position.y + .4 && y >= position.y - .35){ return true;}
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
		if (!endCase)
		{
			position.x -= elapsed * speed / 20;
			if (position.x <= -3.55 * 2 + 5.5 && alive)
			{
				speed = 0.0;
				endCase = true;
			}
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

*/

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
}


void Update(float& elapsed)
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		//entities[i]->Update(elapsed);
	}
}

void Render()
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		//entities[i]->Draw(&textured);
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


		ProcessEvents();
		Update(elapsed);
		Render();

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
