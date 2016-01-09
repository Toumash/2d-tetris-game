#include <cstdlib>
#include "main.h"

#ifndef __TETRIS__
#define __TETRIS__

struct Tetris
{
private:
	int PlayerX;
	int PlayerY;
public:
	char** Matrix;
	char** Player;

	int Width;
	int Height;
	bool canRotate;

	int score = 0, level = 0;
	double worldTime = 0;
	int LastPlayerX;
	int LastPlayerY;

	Tetris(int width, int height)
	{
		this->PlayerX = width / 2 - 2;
		this->PlayerY = 0;
		this->Width = width;
		this->Height = height;

		this->Player = NULL;
		this->canRotate = true;
		Matrix = (char**)malloc(height * sizeof(char*));// new char*[height];
		for (int i = 0; i < height; i++)
		{
			Matrix[i] = (char*)malloc(width* sizeof(char));// new char[width];
		}
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				Matrix[y][x] = I_FREE;
			}
		}
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
				if (PlayerY - SHAPE_PIVOT + y < Height && Player[y][x] != I_FREE) {
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
