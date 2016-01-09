#include <cstdlib>
#include "main.h"


struct Tetris
{
	char** Matrix;
	char** Player;
	int PlayerX;
	int PlayerY;
	int Width;
	int Height;
	bool canRotate;

	Tetris(int width, int height)
	{
		this->PlayerX = width / 2 - 2;
		this->PlayerY = 0;
		this->Width = width;
		this->Height = height;
		Matrix = new char*[height];
		for (int i = 0; i < height; i++)
		{
			Matrix[i] = new char[width];
		}
		for (int y = 2; y < Height; y++)
		{
			for (int x = 0; x <Width; x++)
			{
				Matrix[y][x] = I_FREE;
			}
		}
	}

	bool isColliding(char** player, int playerX, int playerY) const
	{
		/*for (int y = 0; y < Height;y++)
		{
			for (int x = 0; x < Width; x++)
			{
				if(player[y][x] != I_FREE && Matrix[SHAPE_PIVOT - y + playerY][SHAPE_PIVOT - x + playerX] != I_FREE)
				{
					return true;
				}
			}
		}*/
		return false;
	}

	void PutShape()
	{
		
	}

	void DisposePlayer() const
	{
		for (int i = 0; i < SHAPE_SIZE; i++)
		{
			free(Player[i]);
		}
		free(Player);
	}
	void Dispose() const
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
};
