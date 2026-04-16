#include "game/GameSession.h"

GameSession::GameSession(const std::string& id, const std::vector<Player>& players)
    : roomId(id),
    players(players),
    currentTurnIndex(0),
    finished(false)
{
    isSpectator.resize(players.size(), false);
}

bool GameSession::IsPlayerTurn(sf::TcpSocket* socket) const
{
    return players[currentTurnIndex].GetSocket() == socket;
}


bool GameSession::MakeMove(sf::TcpSocket* socket, int row, int col)
{
    if (finished)
        return false;

    if (!IsPlayerTurn(socket))
        return false;

    int playerIndex = currentTurnIndex;

    // mapear jugador -> ficha
    Cell cell = static_cast<Cell>(playerIndex + 1);

    if (!board.MakeMove(cell, row, col))
        return false;

    // comprobar victoria
    if (board.CheckWin(cell))
    {
        winners.push_back(players[playerIndex].GetUsername());
        isSpectator[playerIndex] = true;

        if (winners.size() >= 3)
        {
            finished = true;
        }
    }

    AdvanceTurn();

    return true;
}

void GameSession::AdvanceTurn()
{
    int count = players.size();

    for (int i = 0; i < count; i++)
    {
        currentTurnIndex = (currentTurnIndex + 1) % count;

        if (!isSpectator[currentTurnIndex])
            return;
    }
}