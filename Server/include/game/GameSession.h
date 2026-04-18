#pragma once
#include <vector>
#include <string>
#include <random>

#include "network/Player.h"
#include "game/GameBoard.h"

class GameSession
{
private:
    std::string roomId;

    std::vector<Player> players;
    std::vector<bool> isSpectator;

    GameBoard board;

    int currentTurnIndex;
    std::vector<std::string> winners;

    bool finished;

public:
    GameSession(const std::string& id, const std::vector<Player>& players);

    bool MakeMove(sf::TcpSocket* socket, int row, int col, Cell& cell);

    void AdvanceTurn();

    bool IsPlayerTurn(sf::TcpSocket* socket) const;
    
    bool IsFinished() const { return finished; }
    const GameBoard& GetBoard() const { return board; };
    const std::vector<Player> GetPlayers() const { return players; };

    bool HasPlayer(sf::TcpSocket* socket) const;

    void AssignColors();
};