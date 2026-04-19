#pragma once

#include <vector>

#include "core/Constants.h"

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
public:
    /** Builds an empty board ready for a new match. */
    GameBoard();

    /** Tries to place a token on the requested board cell. */
    bool MakeMove(Cell cell, int row, int column);

    /** Returns a mutable reference to a board cell. */
    Cell& GetCell(int row, int column);

    /** Returns a read-only reference to a board cell. */
    const Cell& GetCell(int row, int column) const;

    /** Counts consecutive cells in one direction. */
    int CountDirection(int row, int column, int deltaRow, int deltaColumn, Cell cell) const;

    /** Returns true when the board is completely filled. */
    bool CheckDraw() const;

    /** Returns true when a move creates a winning line. */
    bool CheckWin(int row, int column, Cell cell) const;

    /** Clears the board state for a new match. */
    void Reset();

private:
    /** Checks whether the requested coordinates are inside the board. */
    bool IsInsideBoard(int row, int column) const;

    std::vector<Cell> board;
};
