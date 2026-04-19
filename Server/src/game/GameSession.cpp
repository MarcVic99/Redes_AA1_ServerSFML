#include "game/GameSession.h"

#include <algorithm>
#include <array>
#include <iostream>

GameSession::GameSession(const std::string& roomIdValue, const std::vector<Player>& sessionPlayers)
    : roomId(roomIdValue),
      players(sessionPlayers),
      currentTurnIndex(0),
      finished(false),
      draw(false)
{
    isSpectator.resize(players.size(), false);
    losers = players;

    AssignColors();
    turnClock.restart();
}

bool GameSession::IsPlayerTurn(sf::TcpSocket* socket) const
{
    return currentTurnIndex < players.size() && players[currentTurnIndex].GetSocket() == socket;
}

bool GameSession::HasPlayer(sf::TcpSocket* socket) const
{
    for (const Player& player : players)
    {
        if (player.GetSocket() == socket)
        {
            return true;
        }
    }

    return false;
}

void GameSession::AssignColors()
{
    std::array<PlayerColor, kMaxPlayers> colors{
        PlayerColor::Rojo,
        PlayerColor::Naranja,
        PlayerColor::Verde,
        PlayerColor::Azul,
    };

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(colors.begin(), colors.end(), generator);

    for (std::size_t index = 0; index < players.size(); ++index)
    {
        players[index].SetPlayerColor(colors[index]);
    }
}

bool GameSession::SessionMakeMove(sf::TcpSocket* socket, int row, int column, Cell& cell)
{
    if (finished || !IsPlayerTurn(socket))
    {
        return false;
    }

    if (isSpectator[currentTurnIndex])
    {
        std::cout << "Player " << players[currentTurnIndex].GetUsername() << " already won." << std::endl;
        AdvanceTurn();
        RestartTurnClock();
        return false;
    }

    const std::size_t playerIndex = currentTurnIndex;
    cell = static_cast<Cell>(playerIndex + 1);

    if (!board.MakeMove(cell, row, column))
    {
        return false;
    }

    if (board.CheckWin(row, column, cell))
    {
        winners.push_back(players[playerIndex]);

        for (auto loserIterator = losers.begin(); loserIterator != losers.end(); ++loserIterator)
        {
            if (loserIterator->GetUsername() == players[playerIndex].GetUsername())
            {
                losers.erase(loserIterator);
                break;
            }
        }

        isSpectator[playerIndex] = true;

        if (winners.size() >= kWinnerPoints.size())
        {
            finished = true;
        }
    }

    if (!finished && board.CheckDraw())
    {
        draw = true;
        finished = true;
        std::cout << "La partida termina en empate." << std::endl;
    }

    if (!finished)
    {
        AdvanceTurn();
        RestartTurnClock();
    }

    return true;
}

void GameSession::AdvanceTurn()
{
    const std::size_t playerCount = players.size();

    for (std::size_t turnCount = 0; turnCount < playerCount; ++turnCount)
    {
        currentTurnIndex = (currentTurnIndex + 1) % playerCount;

        if (!isSpectator[currentTurnIndex])
        {
            return;
        }
    }
}

void GameSession::RestartTurnClock()
{
    turnClock.restart();
}

bool GameSession::HasTurnTimedOut() const
{
    return turnClock.getElapsedTime().asSeconds() >= kTurnLimitSeconds;
}

bool GameSession::UpdateTurnTimeout()
{
    if (finished || !HasTurnTimedOut())
    {
        return false;
    }

    std::cout << "Turno agotado para jugador: " << players[currentTurnIndex].GetUsername() << std::endl;

    AdvanceTurn();
    RestartTurnClock();

    return true;
}
