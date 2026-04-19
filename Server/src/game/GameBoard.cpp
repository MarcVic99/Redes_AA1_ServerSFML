#include "game/GameBoard.h"

GameBoard::GameBoard()
{
    board.assign(NUM_ROWS * NUM_COLUMNS, Cell::Empty);
}

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
int GameBoard::CountDirection(int row, int col, int dRow, int dCol, Cell cell) {
    int count = 0;

    int r = row + dRow;
    int c = col + dCol;

    while (r >= 0 && r < NUM_ROWS && c >= 0 && c < NUM_COLUMNS &&
        GetCell(r, c) == cell) {
        count++;
        r += dRow;
        c += dCol;
    }

    return count;
}
Cell& GameBoard::GetCell(int row, int column)
{
	return board[row * NUM_COLUMNS + column];

}

bool GameBoard::CheckWin(int row, int col, Cell cell) {
    // Horizontal
    if (1 + CountDirection(row, col, 0, 1, cell)
        + CountDirection(row, col, 0, -1, cell) >= 3)
        return true;

    // Vertical
    if (1 + CountDirection(row, col, 1, 0, cell)
        + CountDirection(row, col, -1, 0, cell) >= 3)
        return true;

    // Diagonal derecha
    if (1 + CountDirection(row, col, 1, 1, cell)
        + CountDirection(row, col, -1, -1, cell) >= 3)
        return true;

    // Diagonal izquierda
    if (1 + CountDirection(row, col, 1, -1, cell)
        + CountDirection(row, col, -1, 1, cell) >= 3)
        return true;

    return false;
}

bool GameBoard::CheckDraw() const
{
    for (const auto& cell : board)
    {
        if (cell == Empty)
        {
            return false;
        }
    }
    return true;
}

void GameBoard::Reset()
{
}
