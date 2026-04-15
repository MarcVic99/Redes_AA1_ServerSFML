#pragma once
#include <vector>
#include "Player.h"
#include <string>

class Room {
    std::string id;
    std::vector<Player> players;
    static const int MAX_PLAYERS = 4;

public:
	int GetMaxPlayers() const { return MAX_PLAYERS; }

    const std::string GetId() const { return id; }
    void SetId(const std::string& newId) { id = newId; }
    
    const std::vector<Player>& GetPlayers() const { return players; }


    bool IsFull() const { return players.size() >= MAX_PLAYERS; }

    bool HasPlayer();

    bool AddPlayer(sf::TcpSocket* _socket, const std::string& _username);

    void RemovePlayer();
};