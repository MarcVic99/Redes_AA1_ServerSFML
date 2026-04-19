#include "game/GameBoard.h"

#include <iostream>

GameBoard::GameBoard()
{
    Reset();
}

bool GameBoard::MakeMove(Cell cell, int row, int column)
{
    if (!IsInsideBoard(row, column))
    {
        std::cout << "Invalid cell. Coordinates are outside the board" << std::endl;
        return false;
    }

    if (GetCell(row, column) != Cell::Empty)
    {
        std::cout << "Invalid cell. Cell is not empty" << std::endl;
        return false;
    }

    GetCell(row, column) = cell;
    return true;
}

Cell& GameBoard::GetCell(int row, int column)
{
    return board[static_cast<std::size_t>(row * kBoardColumns + column)];
}

const Cell& GameBoard::GetCell(int row, int column) const
{
    return board[static_cast<std::size_t>(row * kBoardColumns + column)];
}

int GameBoard::CountDirection(int row, int column, int deltaRow, int deltaColumn, Cell cell) const
{
    int count = 0;
    int currentRow = row + deltaRow;
    int currentColumn = column + deltaColumn;

    while (IsInsideBoard(currentRow, currentColumn) && GetCell(currentRow, currentColumn) == cell)
    {
        ++count;
        currentRow += deltaRow;
        currentColumn += deltaColumn;
    }

    return count;
}

bool GameBoard::CheckWin(int row, int column, Cell cell) const
{
    if (1 + CountDirection(row, column, 0, 1, cell) + CountDirection(row, column, 0, -1, cell) >= kWinningLineLength)
    {
        return true;
    }

    if (1 + CountDirection(row, column, 1, 0, cell) + CountDirection(row, column, -1, 0, cell) >= kWinningLineLength)
    {
        return true;
    }

    if (1 + CountDirection(row, column, 1, 1, cell) + CountDirection(row, column, -1, -1, cell) >= kWinningLineLength)
    {
        return true;
    }

    if (1 + CountDirection(row, column, 1, -1, cell) + CountDirection(row, column, -1, 1, cell) >= kWinningLineLength)
    {
        return true;
    }

    return false;
}

bool GameBoard::CheckDraw() const
{
    for (const Cell cell : board)
    {
        if (cell == Cell::Empty)
        {
            return false;
        }
    }

    return true;
}

void GameBoard::Reset()
{
    board.assign(static_cast<std::size_t>(kBoardRows * kBoardColumns), Cell::Empty);
}

bool GameBoard::IsInsideBoard(int row, int column) const
{
    return row >= 0 && row < kBoardRows && column >= 0 && column < kBoardColumns;
}
