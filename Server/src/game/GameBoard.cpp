#include "game/GameBoard.h"

bool GameBoard::MakeMove(Cell cell, int row, int column)
{
	
}

Cell GameBoard::GetCell(int row, int column) const
{
	return board[row * NUM_COLUMNS + column];
}

bool GameBoard::CheckWin(Cell cell)
{
	return false;
}

void GameBoard::Reset()
{
}
