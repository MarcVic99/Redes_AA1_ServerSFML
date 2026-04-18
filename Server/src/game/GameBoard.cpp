#include "game/GameBoard.h"

bool GameBoard::MakeMove(Cell cell, int row, int column)
{

    if (GetCell(row, column) != Cell::Empty)
    {
        std::cout << "Invalid Cell. Cell is not Empty" << std::endl;
        return false;
    }

    GetCell(row, column) = cell;
    return true;

}

Cell& GameBoard::GetCell(int row, int column)
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
