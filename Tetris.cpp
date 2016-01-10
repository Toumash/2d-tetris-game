#include <cstdlib>
#include "main.h"
#include <cstdio>

#ifndef __TETRIS__
#define __TETRIS__

struct Tetris
{
private:
	int PlayerX;
	int PlayerY;
public:
	// standard tetris dimensions
	static const int WIDTH = 10;
	static const int HEIGHT = 22;
	char** Matrix;
	char** Player;

	int Width;
	int Height;
	bool canRotate;

	int score = 0, level = 0;
	double worldTime = 0;
	int LastPlayerX = 0;
	int LastPlayerY = 0;
	int saves = 0;

	Tetris(int width, int height)
	{
		ResetPlayerPosition();
		this->Width = width;
		this->Height = height;

		this->Player = NULL;
		this->canRotate = true;
		Matrix = (char**)malloc(height * sizeof(char*));
		for (int y = 0; y < height; y++)
		{
			Matrix[y] = (char*)malloc(width* sizeof(char));
			for (int x = 0; x < width; x++)
			{
				Matrix[y][x] = I_FREE;
			}
		}
	}
	void ResetPlayerPosition()
	{
		PlayerX = WIDTH / 2 - 2;
		PlayerY = 0;
	}
	/*worldTime score level saves
	width height
	map
	playerX playerY
	lastX lastY
	playerShape
*/
	static bool LoadFromFile(Tetris* game, const char * name)
	{
		FILE* f = fopen(name, "r");
		if (!f)
		{
			return false;
		}
		double wT;
		int score, level, saves;
		fscanf(f, "%lf %d %d %d", &wT, &score, &level, &saves);
		int width, height;
		fscanf(f, "%d %d", &width, &height);
		game->worldTime = wT;
		game->score = score;
		game->level = level;
		game->saves = saves;
		game->Width = width;
		game->Height = height;
		game->Matrix = (char**)malloc(height + 2 * sizeof(char*));
		int tmp;
		for (int y = 0; y < height; y++)
		{
			game->Matrix[y] = (char*)malloc(width* sizeof(char));
			for (int x = 0; x < width; x++)
			{
				fscanf(f, "%d", &tmp);
				game->Matrix[y][x] = (char)tmp;
			}
		}

		fscanf(f, "%d %d", &game->PlayerX, &game->PlayerY);
		fscanf(f, "%d %d", &game->LastPlayerX, &game->LastPlayerY);
		game->Player = (char**)malloc(SHAPE_SIZE * sizeof(char*));
		for (int y = 0; y < SHAPE_SIZE; y++)
		{
			game->Player[y] = (char*)malloc(SHAPE_SIZE * sizeof(char));
			for (int x = 0; x < SHAPE_SIZE; x++)
			{
				fscanf(f, "%d", &tmp);
				game->Player[y][x] = tmp;
			}
		}
		fclose(f);
		f = NULL;
		return true;
	}
	static bool SaveGame(const char * saveFile, Tetris& game)
	{
		FILE* f = fopen(saveFile, "w+");
		if (!f)
		{
			return false;
		}

		fprintf(f, "%f %i %i %i\n", game.worldTime, game.score, game.level, game.saves);
		fprintf(f, "%i %i\n", game.Width, game.Height);
		for (int y = 0; y < game.Height; y++)
		{
			for (int x = 0; x < game.Width; x++)
			{
				fprintf(f, "%i ", game.Matrix[y][x]);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "%i %i\n", game.PlayerX, game.PlayerY);
		fprintf(f, "%i %i\n", game.LastPlayerX, game.LastPlayerY);
		for (int y = 0; y < SHAPE_SIZE; y++)
		{
			for (int x = 0; x < SHAPE_SIZE; x++)
			{
				fprintf(f, "%i ", game.Player[y][x]);
			}
			fprintf(f, "\n");
		}
		fflush(f);
		fclose(f);
		return true;
	}

	int GetPlayerX() const
	{
		return this->PlayerX;
	}
	int GetPlayerY() const
	{
		return this->PlayerY;
	}
	void SetPlayerX(int x)
	{
		LastPlayerX = PlayerX;
		PlayerX = x;
	}
	void SetPlayerY(int y)
	{
		LastPlayerY = PlayerY;
		PlayerY = y;
	}
	bool isColliding(int dstX, int dstY, char** player) const
	{
		char** matrix = this->Matrix;
		int height = this->Height;
		int width = this->Width;

		for (int y = 0; y < SHAPE_SIZE; y++)
		{
			for (int x = 0; x < SHAPE_SIZE; x++)
			{
				if (player[y][x] != I_FREE) {
					// checks agains bounds (canvas)
					if ((y - SHAPE_PIVOT + dstY >= height)
						|| (x - SHAPE_PIVOT + dstX >= width)
						|| (x - SHAPE_PIVOT + dstX < 0))
					{
						return true;
					}
					// checks agains other fields
					if (dstY - SHAPE_PIVOT + y >= 2 && dstX - SHAPE_PIVOT + x >= 0) {
						if (matrix[dstY - SHAPE_PIVOT + y][dstX - SHAPE_PIVOT + x] != I_FREE)
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	// returns the amount of deleted lines
	int DeleteFullLines()
	{
		int deletedAmount = 0;
		for (int row = Height - 1; row > 0; row--)
		{
			bool full = true;
			for (int x = 0; x < Width; x++)
			{
				if (Matrix[row][x] == I_FREE)
				{
					full = false;
				}
			}
			if (full)
			{
				deletedAmount++;
				//removing process
				// Moves all the upper lines one row down
				for (int y = row; y > 0; y--)
				{
					for (int x = 0; x < Width; x++)
					{
						Matrix[y][x] = Matrix[y - 1][x];
					}
				}
			}
		}
		return deletedAmount;
	}
	int previouslyRemovedRows = 0;
	void PlaceTetronimo()
	{
		for (int y = 0; y < SHAPE_SIZE; y++)
		{
			for (int x = 0; x < SHAPE_SIZE; x++)
			{
				if (PlayerY - SHAPE_PIVOT + y >= 0 && PlayerY - SHAPE_PIVOT + y < Height && Player[y][x] != I_FREE) {
					Matrix[PlayerY - SHAPE_PIVOT + y][PlayerX - SHAPE_PIVOT + x] = Player[y][x];
				}
			}
		}
		// calculate point
		int deletedRows = DeleteFullLines();
		score += calculatePoints(level, deletedRows, previouslyRemovedRows);
		previouslyRemovedRows = deletedRows;
	}

	static int calculatePoints(int level, int removedAmount, int previousRemoved)
	{
		switch (removedAmount)
		{
		case 1:
			return 100 * (level + 1);
		case 2:
			return 200 * (level + 1);
		case 3:
			return 400 * (level + 1);
		case 4:
			return (previousRemoved > 4 ? 1200 : 800)*(level + 1);
		default:
			return 0;
		}
	}

	void DisposePlayer()
	{
		if (Player != NULL) {
			for (int i = 0; i < SHAPE_SIZE; i++)
			{
				free(Player[i]);
			}
			free(Player);
		}
	}
	void Dispose()
	{
		for (int i = 0; i < Height; i++)
		{
			free(Matrix[i]);
		}
		free(Matrix);
	}

	bool CanRotate() const
	{
		return this->canRotate;
	}

	void NextShape(char** new_shape)
	{
		DisposePlayer();
		this->Player = new_shape;
	}

	bool IsOver() const
	{
		bool over = false;
		for (int x = 0; x < Width; x++)
		{
			if (Matrix[2][x] != I_FREE)
			{
				over = true;
			}
		}
		return over;
	}
};
#endif
