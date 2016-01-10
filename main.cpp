#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <SDL.h>
#include "Tetris.cpp"
#include "main.h"
#include <ctime>

extern "C" {
	//#ifdef BIT64
	//#include"./sdl64/include/SDL.h"
	//#include"./sdl64/include/SDL_main.h"
	//#else
#include"./sdl/include/SDL_main.h"
//#endif
}

const int LEVEL_DURATION = 20;
const int MAX_LEVEL = 10;
const int MAX_GAME_SAVES = 3;


const int FPS_LIMIT = 40;
// determines how much ms to wait between logic updates
const double LOGIC_STEP = 1000 / 20;
const double FRAME_TIME = 1000 / FPS_LIMIT;


const int SCREEN_WIDTH = 640;
const int  SCREEN_HEIGHT = 480;
const bool FULLSCREEN = false;
const int BLOCK_SIZE = 20;
const int PADDING_X = 200;
const int PADDING_Y = 22;
int outLineColor = I_BLACK;


const char * DEFAULT_SAVE = "tetris.save";


int Colors[7];
char Pieces[S_AMOUNT][SHAPE_SIZE][SHAPE_SIZE] = {
	{ // blue I
		{ 0,0,0,0,0 },
		{ 0,0,I_BLUE,0,0 },
		{ 0,0,I_BLUE,0,0 },
		{ 0,0,I_BLUE,0,0 },
		{ 0,0,I_BLUE,0,0 }
	}
	,{// red cube
		{ 0,0,0,0,0 },
		{ 0,0,0,0,0 },
		{ 0,0,I_RED,I_RED,0 },
		{ 0,0,I_RED,I_RED,0 },
		{ 0,0,0,0,0 }
	}
	,{// green Z
		{ 0,0,0,0,0 },
		{ 0,I_GREEN,I_GREEN,0,0 },
		{ 0,0,I_GREEN,I_GREEN,0 },
		{ 0,0,0,0,0 },
		{ 0,0,0,0,0 }
	}
	,{ // reverse Z
		{ 0,0,0,0,0 },
		{ 0,0,I_GREEN,I_GREEN,0 },
		{ 0,I_GREEN,I_GREEN,0,0 },
		{ 0,0,0,0,0 },
		{ 0,0,0,0,0 }
	},
	{ // white L
		{ 0,0,0,0,0 },
		{ 0,0,I_WHITE,0,0 },
		{ 0,0,I_WHITE,0,0 },
		{ 0,0,I_WHITE,I_WHITE,0 },
		{ 0,0,0,0,0 },
	},
	{ // reverse L
		{ 0,0,0,0,0 },
		{ 0,0,I_WHITE,0,0 },
		{ 0,0,I_WHITE,0,0 },
		{ 0,I_WHITE,I_WHITE,0,0 },
		{ 0,0,0,0,0 },
	},{
		//black t
		{ 0,0,0,0,0 },
		{ 0,0,I_LIONESS,0,0 },
		{ 0,I_LIONESS,I_LIONESS,I_LIONESS,0 },
		{ 0,0,0,0,0 },
		{ 0,0,0,0,0 }
	}
};

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y);
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawFullRectangle(SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor, Uint32 fillColor);
void DrawRectangle(struct SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor);
char** getShape(int index);
char** rotate(char** array, int size);
void nextShape(Tetris& game);
void DrawPlayer(SDL_Surface* screen, Tetris& game, double dt, bool fastDrop, Uint32 currentTime, Uint32& lastDropTime);
int getBlockDelay(int level);
float Approach(float flGoal, float flCurrent, float dt)
{
	float flDifference = flGoal - flCurrent;
	if (flDifference > dt)
	{
		return flCurrent + dt;
	}
	if (flDifference < -dt)
	{
		return flCurrent - dt;
	}
	return flGoal;
}

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
};

// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};
void DrawFullRectangle(SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, h, 0, 1, outlineColor);
	DrawLine(screen, x + w - 1, y, h, 0, 1, outlineColor);
	DrawLine(screen, x, y, w, 1, 0, outlineColor);
	DrawLine(screen, x, y + h - 1, w, 1, 0, outlineColor);
	for (i = y + 1; i < y + h - 1; i++)
		DrawLine(screen, x + 1, i, w - 2, 1, 0, fillColor);
};
void DrawRectangle(SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor) {
	DrawLine(screen, x, y, h, 0, 1, outlineColor);
	DrawLine(screen, x + w - 1, y, h, 0, 1, outlineColor);
	DrawLine(screen, x, y, w, 1, 0, outlineColor);
	DrawLine(screen, x, y + h - 1, w, 1, 0, outlineColor);
};

void nextShape(Tetris& game)
{
	int piece = rand() % S_AMOUNT;
	char** newShape = getShape(piece);
	game.NextShape(newShape);
	game.canRotate = piece != S_Square;
	game.SetPlayerX(game.Width / 2 - 1);
	game.SetPlayerY(0);
}
int getBlockDelay(int level)
{
	return 800 - level * 70;
}

bool FASTDROP_STYLE_TELEPORT = true;
void update(Tetris &game, Uint32 currentTime, Uint32& lastDrop, double worldTime, double& lastLevelTime, bool& fastDrop)
{

	if (worldTime - lastLevelTime > LEVEL_DURATION && game.level < MAX_LEVEL)
	{
		game.level += 1;
		lastLevelTime = worldTime;
	}

	int delay = getBlockDelay(game.level);

	if (fastDrop)
	{
		int x = game.GetPlayerX();
		int y = game.GetPlayerY();
		if (FASTDROP_STYLE_TELEPORT)
		{
			while (!game.isColliding(x, y + 1, game.Player))
			{
				y += 1;
			}
			game.SetPlayerY(y);
			game.PlaceTetronimo();
			nextShape(game);
			fastDrop = false;
		}
		else {
			if (game.isColliding(x, y + 2, game.Player))
			{
				if (game.isColliding(x, y + 1, game.Player))
				{
					game.PlaceTetronimo();
					nextShape(game);
					fastDrop = false;
				}
				else
				{
					game.SetPlayerY(y + 1);
				}
			}
			else
			{
				if (game.isColliding(x, y + 1, game.Player))
				{
					game.PlaceTetronimo();
					nextShape(game);
					fastDrop = false;
				}
				else
				{
					game.SetPlayerY(y + 2);
				}
			}
		}

	}
	else if (currentTime - lastDrop > (unsigned int)delay) {

		if (game.isColliding(game.GetPlayerX(), game.GetPlayerY() + 1, game.Player))
		{
			game.PlaceTetronimo();
			nextShape(game);
			fastDrop = false;
		}
		else {
			game.SetPlayerY(game.GetPlayerY() + 1);
		}
		lastDrop = currentTime;
	}
}

void restartGame(Tetris& game)
{
	game.DisposePlayer();
	game.Dispose();
	game = Tetris(game.Width, game.Height);
	nextShape(game);
}

void handleInput(Tetris &game, SDL_Event &event, bool &quit, bool& pause, Uint32 currentTime, Uint32& lastDrop, bool& debug, double& lastLevelTime, bool& fastDrop)
{
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
			break;
		case SDL_KEYUP:

			switch (event.key.keysym.sym)
			{
				// rotation
			case SDLK_UP:case SDLK_SPACE:
			{
				if (game.CanRotate()) {
					char** player = rotate(game.Player, SHAPE_SIZE);
					if (!game.isColliding(game.GetPlayerX(), game.GetPlayerY(), player)) {
						game.DisposePlayer();
						game.Player = player;
					}
					else
					{
						// abandon new block
						for (int i = 0; i < SHAPE_SIZE; i++)
						{
							free(player[i]);
						}
						free(player);
					}
				}
				break;
			}
			case SDLK_DOWN:
				if (game.GetPlayerY() >= 2) {
					fastDrop = true;
				}
				break;
			case SDLK_RIGHT:
				if (!game.isColliding(game.GetPlayerX() + 1, game.GetPlayerY(), game.Player))
				{
					game.SetPlayerX(game.GetPlayerX() + 1);
				}
				break;

			case SDLK_LEFT:
				if (!game.isColliding(game.GetPlayerX() - 1, game.GetPlayerY(), game.Player))
				{
					game.SetPlayerX(game.GetPlayerX() - 1);
				}
				break;
			case SDLK_p:
				pause = !pause;
				break;
			case SDLK_w:
				if (game.level < MAX_LEVEL) {
					game.level += 1;
					lastLevelTime = game.worldTime;
				}
				break;
			case SDLK_s:
				if(game.saves < MAX_GAME_SAVES)
				{
					game.saves += 1;
					Tetris::SaveGame(DEFAULT_SAVE, game);
				}
				break;
			case SDLK_l:
					Tetris::LoadFromFile(&game, DEFAULT_SAVE);
				break;
			case SDLK_n:
				restartGame(game);
				break;
			case SDLK_q:
				debug = !debug;
				break;
			case SDLK_t:
				FASTDROP_STYLE_TELEPORT = !FASTDROP_STYLE_TELEPORT;
				break;
			default:
				break;
			}

			break;
		case SDL_QUIT:
			quit = true;
			break;
		};
	}
}

void DrawPlayer(SDL_Surface* screen, Tetris& game, double dt, bool fastDrop, Uint32 currentTime, Uint32& lastDropTime)
{
	//pivotX = game.GetPlayerX();// -(game.LastPlayerX - game.GetPlayerX()) * dt;
	//pivotY = Approach(game.GetPlayerY(),game.LastPlayerY,(getBlockDelay(game.level) - (currentTime - lastDropTime)) / getBlockDelay(game.level));
	int pivotX = game.GetPlayerX();
	int pivotY = game.GetPlayerY();
	for (int y = 0; y < SHAPE_SIZE; y++)
	{
		for (int x = 0; x < SHAPE_SIZE; x++)
		{
			if (game.Player[y][x] != I_FREE)
			{
				if (pivotY - SHAPE_PIVOT + y - 2 >= 0)
					DrawFullRectangle(screen, PADDING_X + (x + pivotX - SHAPE_PIVOT)* BLOCK_SIZE,
						PADDING_Y + (y + pivotY - SHAPE_PIVOT - 2) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, outLineColor, Colors[game.Player[y][x]]);
			}
		}
	}
}

void render(SDL_Surface* screen, Tetris& game, double dt, bool fastDrop, Uint32 currentTime, Uint32& lastDropTime)
{
	// 2 fields hidden (shhh...)
	for (int y = 2; y < game.Height; y++)
	{
		for (int x = 0; x < game.Width; x++)
		{
			if (game.Matrix[y][x] == I_FREE)
			{
				DrawRectangle(screen, PADDING_X + x * BLOCK_SIZE, PADDING_Y + (y - 2) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, Colors[outLineColor]);
			}
			else {
				DrawFullRectangle(screen, PADDING_X + x* BLOCK_SIZE, PADDING_Y + (y - 2) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, Colors[outLineColor], Colors[game.Matrix[y][x]]);
			}
		}
	}
	DrawPlayer(screen, game, dt, fastDrop, currentTime, lastDropTime);

	// top border 
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y - BLOCK_SIZE, (game.Width + 2) * BLOCK_SIZE, BLOCK_SIZE, Colors[outLineColor], Colors[I_LIGHTGREEN]);
	// left
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y, BLOCK_SIZE, (game.Height - 2 + 1) * BLOCK_SIZE, Colors[outLineColor], Colors[I_LIGHTGREEN]);
	// bottom
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y + (game.Height - 2) * BLOCK_SIZE, (game.Width + 2) * BLOCK_SIZE, BLOCK_SIZE, Colors[outLineColor], Colors[I_LIGHTGREEN]);
	// right
	DrawFullRectangle(screen, PADDING_X + game.Width * BLOCK_SIZE, PADDING_Y, BLOCK_SIZE, (game.Height - 2 + 1) * BLOCK_SIZE, Colors[outLineColor], Colors[I_LIGHTGREEN]);
}

char** rotate(char** array, int size)
{
	char** newArr = (char**)malloc(size * sizeof(char*));
	for (int i = 0; i < size; i++)
	{
		newArr[i] = (char*)malloc(size * sizeof(char));
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			newArr[i][j] = array[size - 1 - j][i];
		}
	}
	return newArr;
}

char** getShape(int index)
{
	char** newArr = (char**)malloc(SHAPE_SIZE * sizeof(char*));
	for (int i = 0; i < SHAPE_SIZE; i++)
	{
		newArr[i] = (char*)malloc(SHAPE_SIZE * sizeof(char));
	}

	for (int y = 0; y < SHAPE_SIZE; y++)
	{
		for (int x = 0; x < SHAPE_SIZE; x++)
		{
			newArr[y][x] = Pieces[index][y][x];
		}
	}
	return newArr;
}

char stringBuffer[128];
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	{
		int rc;
		if (FULLSCREEN) { rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer); }
		else { rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer); }

		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 1;
		};
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Tetris Tomasz Dluski 160741");
	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	{
		// check I_* constans
		Colors[0] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
		Colors[1] = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
		Colors[2] = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
		Colors[3] = SDL_MapRGB(screen->format, 0, 73, 156);
		Colors[4] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
		Colors[5] = SDL_MapRGB(screen->format, 0xE8, 0xD6, 0x97);
		Colors[6] = SDL_MapRGB(screen->format, 76, 153, 0);
	}

	{
		charset = SDL_LoadBMP("./font.bmp");
		if (charset == NULL) {
			printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};
		SDL_SetColorKey(charset, true, 0x000000);
	}
	// standard tetris dimensions
	const int WIDTH = 10;
	const int HEIGHT = 22;
	Tetris game = Tetris(Tetris::WIDTH, Tetris::HEIGHT);
	srand((unsigned int)time(NULL));
	nextShape(game);

	Uint32 currentTime = SDL_GetTicks(), lastTime;
	double deltaS;

	Uint32 renderTime = 0, deltaMs;
	int fps = 0, frames = 0;
	double fpsTimer = 0;

	int updatesPerSecond = 0;
	int logicUpdates = 0;
	Uint32 lastDropTime = currentTime;
	double lastUpdate = lastDropTime;
	bool quit = false;
	bool pause = false;
	bool debug = false;
	double lastLevelTime = 0;
	bool fastDrop = false;

	while (!quit) {
		lastTime = SDL_GetTicks();
		deltaMs = lastTime - currentTime;
		deltaS = deltaMs * 0.001;

		currentTime = lastTime;
		game.worldTime += deltaS;

		handleInput(game, event, quit, pause, currentTime, lastDropTime, debug, lastLevelTime, fastDrop);

		if (!pause) {
			while (currentTime - lastUpdate > LOGIC_STEP) {
				update(game, currentTime, lastDropTime, game.worldTime, lastLevelTime, fastDrop);
				++updatesPerSecond;
				lastUpdate += LOGIC_STEP;
			}
		}

		//background
		SDL_FillRect(screen, NULL, Colors[I_BLACK]);
		// game with player
		render(screen, game, (currentTime - lastUpdate) / LOGIC_STEP, fastDrop, currentTime, lastDropTime);

		//HUD(screen);
		fpsTimer += deltaS;
		if (fpsTimer > 1) {
			fps = frames;
			frames = 0;
			fpsTimer -= 1;

			logicUpdates = updatesPerSecond;
			updatesPerSecond = 0;
		};

		DrawFullRectangle(screen, SCREEN_WIDTH - 200, 4, 190, 50, Colors[I_RED], Colors[I_BLUE]);
		sprintf(stringBuffer, "TETRIS THE GAME");
		DrawString(screen, screen->w - 190, 10, stringBuffer, charset);
		sprintf(stringBuffer, "Poziom: %1i Punkty:%4i", game.level, game.score);
		DrawString(screen, screen->w - 190, 20, stringBuffer, charset);
		sprintf(stringBuffer, "Czas %4.1f", game.worldTime);
		DrawString(screen, screen->w - 190, 30, stringBuffer, charset);
		sprintf(stringBuffer, "Pozostalo zapisow:%i", MAX_GAME_SAVES - game.saves);
		DrawString(screen, screen->w - 190, 40, stringBuffer, charset);

		if (debug) {
			DrawFullRectangle(screen, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 100, 190, 100, Colors[I_RED], Colors[I_BLUE]);
			sprintf(stringBuffer, "%.02i FPS | render: %.02d ms", fps, renderTime);
			DrawString(screen, screen->w - 190, SCREEN_HEIGHT - 80, stringBuffer, charset);

			sprintf(stringBuffer, "%2i,%2i lg: %.02i", game.GetPlayerX(), game.GetPlayerY(), logicUpdates);
			DrawString(screen, screen->w - 190, SCREEN_HEIGHT - 70, stringBuffer, charset);

			sprintf(stringBuffer, "%2.2f", (currentTime - lastUpdate) / LOGIC_STEP);
			DrawString(screen, screen->w - 190, SCREEN_HEIGHT - 60, stringBuffer, charset);

			sprintf(stringBuffer, "last  %i,%i", game.LastPlayerX, game.LastPlayerY);
			DrawString(screen, screen->w - 190, SCREEN_HEIGHT - 50, stringBuffer, charset);
		}

		bool over = game.IsOver();
		if (pause)
		{
			sprintf(stringBuffer, "PAUSED. Click [P] to resume...");
			DrawString(screen, screen->w / 2 - strlen(stringBuffer) * 8 / 2, screen->h / 2, stringBuffer, charset);
		}
		else if (over)
		{
			sprintf(stringBuffer, "Koniec gry. Kliknij T aby zagrac ponownie,N by wyjsc");
			DrawFullRectangle(screen, screen->w / 2 - strlen(stringBuffer) * 8 / 2 - 10, screen->h / 2 - 10, strlen(stringBuffer) * 8 + 30, 50, Colors[I_LIONESS], Colors[I_BLACK]);
			DrawString(screen, screen->w / 2 - strlen(stringBuffer) * 8 / 2, screen->h / 2, stringBuffer, charset);
		}

		// old not accelerated drawing
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		// not supported due to drawing to texture, not renderer
		// SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		if (over)
		{
			while (true) {
				SDL_WaitEvent(&event);
				if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_t)
					{
						restartGame(game);
						// restart game
						over = false;
						break;
					}
					else if (event.key.keysym.sym == SDLK_n)
					{
						quit = true;
						break;
					}
				}
			}
		}

		frames++;
		renderTime = SDL_GetTicks() - lastTime;
		//Sleep the remaining frame time 
		if (renderTime < FRAME_TIME) {
			SDL_Delay((Uint32)(FRAME_TIME - renderTime));
		}
	}

	game.Dispose();
	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};
