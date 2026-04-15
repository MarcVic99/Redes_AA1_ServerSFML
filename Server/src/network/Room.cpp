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

bool Room::RemovePlayer(sf::TcpSocket* socket)
{
    for (auto player = players.begin(); player != players.end(); ++player)
    {
        if (player->GetSocket() == socket)
        {
            std::cout << "Player removed from room " << id << std::endl;

            players.erase(player);
            return true;
        }
    }
    return false;
}


bool Room::HasPlayer(sf::TcpSocket* socket) const
{
    for (const auto& player : players)
    {
        if (player.GetSocket() == socket)
        {
            return true;
        }
    }
    return false;
}

int Room::GetPlayerIndex(sf::TcpSocket* socket) const
{
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].GetSocket() == socket)
            return i;
    }
    return -1;
}