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
		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				Matrix[y][x] = I_FREE;
			}
		}
	}

	bool isColliding(int dstX, int dstY) const
	{
		char** player = this->Player;
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
					if (matrix[dstY - SHAPE_PIVOT + y][dstX - SHAPE_PIVOT + x] != I_FREE)
					{
						return true;
					}
				}
			}
		}
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
