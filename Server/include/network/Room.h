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
    std::string GetId() const { return id; }
	void SetId(std::string newId) { id = newId; }
    const std::vector<Player>& GetPlayers() const { return players; }

    bool AddPlayer(sf::TcpSocket* _socket, const std::string& _username) {
        Player newPlayer = Player(_socket, _username);
        for (auto& player : players)
        {
            if (newPlayer.GetSocket() == player.GetSocket())
                return false; // ya está dentro
        }
        if (players.size() < MAX_PLAYERS) {
            players.push_back(newPlayer);
            return true;
        }
        return false;
    }
};