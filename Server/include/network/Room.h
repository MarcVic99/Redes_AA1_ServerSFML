#pragma once
#include <vector>
#include "Player.h"

#include <string>
#include "core/Constants.h"
#include <iostream>

class Room {
    std::string id;
    std::vector<Player> players;

public:
	int GetMaxPlayers() const { return MAX_PLAYERS; }

    const std::string& GetId() const { return id; }

    void SetId(const std::string& newId) { id = newId; }
    
    const std::vector<Player> GetPlayers() const { return players; }


    bool IsFull() const { return players.size() >= MAX_PLAYERS; }

    bool HasPlayer(sf::TcpSocket* socket) const;

    bool AddPlayer(sf::TcpSocket* _socket, const std::string& _username);

    bool RemovePlayer(sf::TcpSocket* socket);

    int GetPlayerIndex(sf::TcpSocket* socket) const;

};