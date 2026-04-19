#pragma once

#include <random>
#include <string>
#include <vector>

#include "game/GameBoard.h"
#include "network/Player.h"
#include "core/Constants.h"

class GameSession
{
public:
    /** Builds a game session from a room id and its players. */
    GameSession(const std::string& roomId, const std::vector<Player>& players);

    /** Tries to execute a move for the current player. */
    bool SessionMakeMove(sf::TcpSocket* socket, int row, int column, Cell& cell);

    /** Advances the turn to the next active player. */
    void AdvanceTurn();

    /** Returns true when the provided socket belongs to the current turn. */
    bool IsPlayerTurn(sf::TcpSocket* socket) const;

    /** Returns true when the provided socket belongs to the session. */
    bool HasPlayer(sf::TcpSocket* socket) const;

    /** Assigns a random color to each player in the session. */
    void AssignColors();

    /** Updates the turn timeout and skips the turn when needed. */
    bool UpdateTurnTimeout();

    bool IsFinished() const { return finished; }
    const GameBoard& GetBoard() const { return board; }
    const std::vector<Player>& GetPlayers() const { return players; }
    const std::vector<Player>& GetWinners() const { return winners; }
    const std::vector<Player>& GetLosers() const { return losers; }
    std::size_t GetCurrentTurnIndex() const { return currentTurnIndex; }
    const std::string& GetRoomId() const { return roomId; }
    bool IsDraw() const { return draw; }

private:
    /** Returns true when the current turn exceeded the allowed time. */
    bool HasTurnTimedOut() const;

    /** Restarts the clock used to control the turn duration. */
    void RestartTurnClock();

    std::string roomId;
    std::vector<Player> players;
    std::vector<Player> winners;
    std::vector<Player> losers;
    std::vector<bool> isSpectator;
    GameBoard board;
    std::size_t currentTurnIndex;
    sf::Clock turnClock;
    bool finished;
    bool draw;
};
