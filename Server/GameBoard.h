#pragma once

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

	Player actualPlayer;

public:

	GameBoard();

	bool MakeMove(Player* p, int row, int column);
	Cell GetCell(int row, int column) const;

	bool CheckWin(Player* p);
	void Reset();

	//getters
	Player GetCurrentPlayer() { return actualPlayer; }


};

