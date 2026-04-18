#pragma once

#include <iostream>

#include "network/Player.h"

#define NUM_ROWS 6
#define NUM_COLUMNS 6

enum Cell
{
	Empty,
	Player1,
	Player2,
	Player3,
	Player4
};

class GameBoard
{

private:
	//recordad acceder con i(rows) y j(columns)
	std::vector<Cell> board;
	int numCells = NUM_ROWS * NUM_COLUMNS;
public:

	GameBoard();

	bool MakeMove(Cell cell, int row, int column);
	Cell& GetCell(int row, int column);
	int CountDirection(int row, int col, int dRow, int dCol, Cell cell);

	bool CheckWin(int row, int col, Cell cell);
	void Reset();
};

