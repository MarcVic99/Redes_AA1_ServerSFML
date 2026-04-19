#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Player.h"
#include "core/Constants.h"

class Room
{
public:
    /** Returns the maximum number of players allowed in the room. */
    std::size_t GetMaxPlayers() const { return kMaxPlayers; }

    /** Returns the identifier of the room. */
    const std::string& GetId() const { return id; }

    /** Sets the identifier of the room. */
    void SetId(const std::string& newId) { id = newId; }

    /** Returns the players currently stored in the room. */
    const std::vector<Player>& GetPlayers() const { return players; }

    /** Returns true when the room already reached its capacity. */
    bool IsFull() const { return players.size() >= kMaxPlayers; }

    /** Returns true when the provided socket belongs to a room player. */
    bool HasPlayer(sf::TcpSocket* socket) const;

    /** Adds a player to the room if there is free space. */
    bool AddPlayer(sf::TcpSocket* socket, const std::string& username);

    /** Removes a player from the room. */
    bool RemovePlayer(sf::TcpSocket* socket);

    /** Returns the index of a player in the room or -1 if it is missing. */
    int GetPlayerIndex(sf::TcpSocket* socket) const;

private:
    std::string id;
    std::vector<Player> players;
};
