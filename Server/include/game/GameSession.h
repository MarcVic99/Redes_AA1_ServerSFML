#pragma once
#include <vector>
#include <string>
#include <random>

#include "network/Player.h"
#include "game/GameBoard.h"
#include "network/PacketTypes.h"
#include "core/Constants.h"

class GameSession
{
private:
    std::string roomId;

    std::vector<Player> players;
    std::vector<Player> winners;
    std::vector<Player> losers;
    std::vector<bool> isSpectator;

    GameBoard board;

    int currentTurnIndex;

    sf::Clock turnClock;
    
    bool finished;
    bool draw = false;

    bool HasTurnTimedOut() const;
    void RestartTurnClock();

public:

    GameSession(const std::string& id, const std::vector<Player>& players);

    bool SessionMakeMove(sf::TcpSocket* socket, int row, int col, Cell& cell);


    void AdvanceTurn();

    bool IsPlayerTurn(sf::TcpSocket* socket) const;
    
    //Getters
    bool IsFinished() const { return finished; }
    const GameBoard& GetBoard() const { return board; };
    const std::vector<Player> GetPlayers() const { return players; };
    const std::vector<Player> GetWinners() const { return winners; };
    const std::vector<Player> GetLosers() const { return losers; };

	int GetCurrentTurnIndex() const { return currentTurnIndex; }
	bool GetIsFinished() const { return finished; } 
    const std::string& GetRoomId() const { return roomId; }
    bool GetIsDraw() const { return draw; }

    bool HasPlayer(sf::TcpSocket* socket) const;

    void AssignColors();

    bool UpdateTurnTimeout();
};