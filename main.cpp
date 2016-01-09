#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
extern "C" {
	//#ifdef BIT64
	//#include"./sdl64/include/SDL.h"
	//#include"./sdl64/include/SDL_main.h"
	//#else
#include"./sdl/include/SDL_main.h"
//#endif
}
const int FPS_LIMIT = 50;
#include <SDL.h>
#include "Tetris.cpp"
#include "main.h"
#include <ctime>


const int FRAME_TIME = 1000 / FPS_LIMIT;
const int SCREEN_WIDTH = 640;
const int  SCREEN_HEIGHT = 480;
const bool FULLSCREEN = false;
const int BLOCK_SIZE = 20;
int outLineColor = I_LIONESS;



// determines how much ms to wait between logic updates
const double LOGIC_STEP_DEFAULT = 1000 / 1;
const double LOGIC_STEP_DROP = 1000 / 7;


int Colors[6];
const int PADDING_X = 200;
const int PADDING_Y = 20;

void render(SDL_Surface* screen, Tetris& game);
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y);
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawFullRectangle(SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor, Uint32 fillColor);
void DrawRectangle(struct SDL_Surface *screen, int x, int y, int w, int h, Uint32 outlineColor);
void DrawPlayer(SDL_Surface* screen, char** shape, int pivotX, int pivotY);
void render(SDL_Surface* screen, Tetris& game);
char** getShape(int index);
char** rotate(char** array, int size);

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

void handleInput(Tetris &game, SDL_Event &event, bool &quit, bool& pause, double& logic_step)
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
					game.DisposePlayer();
					game.Player = player;
				}
				break;
			}
			case SDLK_DOWN:
				logic_step = LOGIC_STEP_DROP;
				break;
			case SDLK_p:
				pause = !pause;
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

void update(Tetris &game, double& logic_step)
{

	if (game.isColliding(game.PlayerX, game.PlayerY + 1))
	{
		game.PutShape();
		logic_step = LOGIC_STEP_DEFAULT;
	}
	else
	{
		game.PlayerY += 1;
	}
}

void DrawPlayer(SDL_Surface* screen, char** shape, int pivotX, int pivotY)
{
	for (int y = 0; y < SHAPE_SIZE; y++)
	{
		for (int x = 0; x < SHAPE_SIZE; x++)
		{
			if (shape[y][x] != I_FREE)
			{
				DrawFullRectangle(screen, PADDING_X + (x + pivotX - SHAPE_PIVOT)* BLOCK_SIZE, PADDING_Y + (y + pivotY - SHAPE_PIVOT) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, outLineColor, Colors[shape[y][x]]);
			}
		}
	}
}

void render(SDL_Surface* screen, Tetris& game)
{
	// top border 
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y - BLOCK_SIZE, (game.Width + 2) * BLOCK_SIZE, BLOCK_SIZE, outLineColor, Colors[I_LIGHTGREEN]);
	// left
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y, BLOCK_SIZE, (game.Height + 1) * BLOCK_SIZE, outLineColor, Colors[I_LIGHTGREEN]);
	// bottom
	DrawFullRectangle(screen, PADDING_X - BLOCK_SIZE, PADDING_Y + game.Height * BLOCK_SIZE, (game.Width + 2) * BLOCK_SIZE, BLOCK_SIZE, outLineColor, Colors[I_LIGHTGREEN]);
	// right
	DrawFullRectangle(screen, PADDING_X + game.Width * BLOCK_SIZE, PADDING_Y, BLOCK_SIZE, (game.Height + 1) * BLOCK_SIZE, outLineColor, Colors[I_LIGHTGREEN]);

	// 2 fields are hidden (shhh...)
	for (int y = 2; y < game.Height; y++)
	{
		for (int x = 0; x < game.Width; x++)
		{
			if (game.Matrix[y][x] == I_FREE)
			{
				DrawRectangle(screen, PADDING_X + x * BLOCK_SIZE, PADDING_Y + (y - 2) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, outLineColor);
			}
			else {
				DrawFullRectangle(screen, PADDING_X + x* BLOCK_SIZE, PADDING_Y + (y - 2) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, outLineColor, Colors[game.Matrix[y][x]]);
			}
		}
	}
	DrawPlayer(screen, game.Player, game.PlayerX, game.PlayerY);
}

char stringBuffer[128];


const int S_I = 0;
const int S_Square = 1;
const int S_Z = 2;
const int S_RZ = 3;
const int S_L = 4;
const int S_RL = 5;
const int S_T = 6;
const int S_AMOUNT = 7;

char Pieces[7][5][5] = {
		{ // blue I
			{0,0,0,0,0},
			{0,0,I_BLUE,0,0},
			{0,0,I_BLUE,0,0},
			{0,0,I_BLUE,0,0},
			{0,0,I_BLUE,0,0}
		}
	,{// red cube
		{0,0,0,0,0},
		{0,0,0,0,0},
		{0,0,I_RED,I_RED,0},
		{0,0,I_RED,I_RED,0},
		{0,0,0,0,0}
	}
	,{// green Z
		{0,0,0,0,0},
		{0,I_GREEN,I_GREEN,0,0},
		{0,0,I_GREEN,I_GREEN,0},
		{0,0,0,0,0 },
		{0,0,0,0,0}
	}
	,{ // reverse Z
		{0,0,0,0,0},
		{0,0,I_GREEN,I_GREEN,0},
		{0,I_GREEN,I_GREEN,0,0},
		{0,0,0,0,0},
		{0,0,0,0,0}
	},
{ // white L
	{0,0,0,0,0},
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
	{0,0,0,0,0},
	{0,0,I_LIONESS,0,0},
	{0,I_LIONESS,I_LIONESS,I_LIONESS,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
}
};

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
// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {	//double etiSpeed;
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
		Colors[3] = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
		Colors[4] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
		Colors[5] = SDL_MapRGB(screen->format, 0xE8, 0xD6, 0x97);
		Colors[6] = SDL_MapRGB(screen->format, 147, 188, 182);
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
	Tetris game = Tetris(WIDTH, HEIGHT);
	srand(time(NULL));
	int piece = rand() % S_AMOUNT;
	game.Player = getShape(piece);
	game.canRotate = piece != S_Square;
	game.PlayerX = 3;
	game.PlayerY = 10;

	Uint32 currentTime = SDL_GetTicks(), lastTime;
	double deltaS, worldTime;

	int renderTime = 0, deltaMs;
	int fps = 0, frames = 0;
	double fpsTimer = 0;
	bool quit = false, pause = false;
	worldTime = 0;
	int score = 0;
	double logic_step = LOGIC_STEP_DEFAULT;
	long int lastUpdate = SDL_GetTicks();

	while (!quit) {
		lastTime = SDL_GetTicks();
		deltaMs = lastTime - currentTime;
		deltaS = deltaMs * 0.001;

		currentTime = lastTime;
		worldTime += deltaS;

		handleInput(game, event, quit, pause, logic_step);
		int updatesPerFrame = 0;
		if (!pause) {
			while (currentTime - lastUpdate > logic_step) {
				update(game, logic_step);
				updatesPerFrame++;
				lastUpdate += logic_step;
			}
		}

		//background
		SDL_FillRect(screen, NULL, Colors[I_BLACK]);

		// game with player
		render(screen, game);

		//HUD(screen);
		fpsTimer += deltaS;
		if (fpsTimer > 1) {
			fps = frames;
			frames = 0;
			fpsTimer -= 1;
		};
		DrawFullRectangle(screen, SCREEN_WIDTH - 200, 4, 190, 50, Colors[I_RED], Colors[I_BLUE]);
		sprintf(stringBuffer, "%.02i FPS | render: %.02d ms", fps, renderTime);
		DrawString(screen, screen->w -190, 10, stringBuffer, charset);
		sprintf(stringBuffer, "x: %02i y:%02i Logic updates: %.02i",game.PlayerX, game.PlayerY, updatesPerFrame);
		DrawString(screen, screen->w - 190 , 26, stringBuffer, charset);
		if (pause)
		{
			sprintf(stringBuffer, "PAUSED. Click [P] to resume...");
			DrawString(screen, screen->w / 2 - strlen(stringBuffer) * 8 / 2, screen->h / 2, stringBuffer, charset);
		}

		// old not accelerated drawing
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		// not supported due to drawing to texture, not renderer
		// SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		frames++;
		renderTime = SDL_GetTicks() - lastTime;
		//Sleep the remaining frame time 
		if (renderTime < FRAME_TIME) {
			SDL_Delay(FRAME_TIME - renderTime);
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
