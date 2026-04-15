#include "network/Room.h"



bool Room::AddPlayer(sf::TcpSocket* _socket, const std::string& _username) {
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

void Room::RemovePlayer()
{
}


bool Room::HasPlayer()
{
    return false;
}
