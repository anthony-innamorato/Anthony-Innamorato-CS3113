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
	Vector acceleration;
	Vector velocity;
	Vector halfLengths;
	Vector friction;
};

struct Player : public Entity
{
	Player() : Entity(spriteSheet, true) {}
	void Draw(ShaderProgram* program)
	{
		/*
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
		*/
	}

	void Update(float elapsed)
	{
		/*
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
		*/
	}
	bool isColliding(float x, float y) const
	{
		/*
		if (x <= position.x + 2.3 && y <= position.y + .4 && y >= position.y - .35) { return true; }
		return false;
		*/
		return true;
	}
	/*
	float u = 0.0;
	float v = 0.0;
	float width = 3288.0 / 4096;
	float height = 888.0 / 4096;
	float size = .7;
	*/
};

struct Enemy : public Entity
{
	Enemy() {}
	void Draw(ShaderProgram* program)
	{
		/*
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
		*/
	}
	void Update(float elapsed)
	{
		/*
		if (!endCase)
		{
			position.x -= elapsed * speed / 20;
			if (position.x <= -3.55 * 2 + 5.5 && alive)
			{
				speed = 0.0;
				endCase = true;
			}
		}
		*/
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
	/*
	Entity* bullet;
	float u;
	float v;
	float width;
	float height;
	float size;
	*/
};


Player p1;
Enemy enemy;

void placeEntity(const string& type, float x, float y) {
	if (type == "Player")
	{
		p1.position = Vector(x, y, 0);
		p1.halfLengths = Vector(TILE_SIZE / 2, TILE_SIZE / 2, 0);
		p1.acceleration = Vector(0, -1, 0);
		p1.friction = Vector(1, 0, 0);
		entities.push_back(&p1);
	}
	if (type == "Enemy")
	{
		enemy.position = Vector(x, y, 0);
		enemy.halfLengths = Vector(TILE_SIZE / 2, TILE_SIZE / 2, 0);
		entities.push_back(&enemy);
		entities.push_back(&enemy);
	}
}



struct FlareMapEntity {
	std::string type;
	float x;
	float y;
};

class FlareMap {
public:
	FlareMap() {
		mapData = nullptr;
		mapWidth = -1;
		mapHeight = -1;
	}
	~FlareMap() {
		for (int i = 0; i < mapHeight; i++) {
			delete mapData[i];
		}
		delete mapData;
	}

	void Load(const std::string fileName)
	{
		std::ifstream infile(fileName);
		if (infile.fail()) {
			assert(false); // unable to open file
		}
		std::string line;
		while (std::getline(infile, line)) {
			if (line == "[header]") {
				if (!ReadHeader(infile)) {
					assert(false); // invalid file data
				}
			}
			else if (line == "[layer]") {
				ReadLayerData(infile);
			}
			else if (line == "[ObjectsLayer]") {
				ReadEntityData(infile);
			}
		}
	}

	int mapWidth;
	int mapHeight;
	unsigned int **mapData;
	std::vector<FlareMapEntity> entities;

private:

	bool ReadHeader(std::ifstream &stream)
	{
		std::string line;
		mapWidth = -1;
		mapHeight = -1;
		while (std::getline(stream, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			std::getline(sStream, key, '=');
			std::getline(sStream, value);
			if (key == "width") {
				mapWidth = std::atoi(value.c_str());
			}
			else if (key == "height") {
				mapHeight = std::atoi(value.c_str());
			}
		}
		if (mapWidth == -1 || mapHeight == -1) {
			return false;
		}
		else {
			mapData = new unsigned int*[mapHeight];
			for (int i = 0; i < mapHeight; ++i) {
				mapData[i] = new unsigned int[mapWidth];
			}
			return true;
		}
	}
	bool ReadLayerData(std::ifstream &stream)
	{
		std::string line;
		while (getline(stream, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			std::getline(sStream, key, '=');
			std::getline(sStream, value);
			if (key == "data") {
				for (int y = 0; y < mapHeight; y++) {
					getline(stream, line);
					std::istringstream lineStream(line);
					std::string tile;
					for (int x = 0; x < mapWidth; x++) {
						std::getline(lineStream, tile, ',');
						unsigned int val = atoi(tile.c_str());
						if (val > 0) {
							mapData[y][x] = val - 1;
						}
						else {
							mapData[y][x] = 0;
						}
					}
				}
			}
		}
		return true;
	}
	bool ReadEntityData(std::ifstream &stream)
	{
		std::string line;
		std::string type;
		while (getline(stream, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "type") {
				type = value;
			}
			else if (key == "location") {
				std::istringstream lineStream(value);
				std::string xPosition, yPosition;
				getline(lineStream, xPosition, ',');
				getline(lineStream, yPosition, ',');

				FlareMapEntity newEntity;
				newEntity.type = type;
				newEntity.x = std::atoi(xPosition.c_str());
				newEntity.y = std::atoi(yPosition.c_str());
				entities.push_back(newEntity);
				placeEntity(type, newEntity.x, newEntity.y);
			}
		}
		return true;
	}

};


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
	projectionMatrix.SetOrthoProjection(-3.55 * 100, 3.55 * 100, -2.0f * 100, 2.0f * 100, -1.0f * 100, 1.0f * 100);
	textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	textured.SetProjectionMatrix(projectionMatrix);
	textured.SetViewMatrix(viewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flare.Load("myMap.txt");
	spriteSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
	//viewMatrix.Translate(-flare.entities[0].y * 20, -flare.entities[0].x * 20, 0);
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
