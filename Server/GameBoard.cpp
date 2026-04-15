#include "GameBoard.h"

GameBoard::GameBoard()
{
	
}

bool GameBoard::MakeMove(Player* p, int row, int column)
{
	//comprobar jugador
	if (p->GetUsername() != actualPlayer.GetUsername())
		return false;
	//comprobar que la casilla esta libre
	if (GetCell(row, column) != Empty)
		return false;



	return false;
}

Cell GameBoard::GetCell(int row, int column) const
{
	return board[row * NUM_COLUMNS + column];
}

bool GameBoard::CheckWin(Player* p)
{
	return false;
}

void GameBoard::Reset()
{
}
