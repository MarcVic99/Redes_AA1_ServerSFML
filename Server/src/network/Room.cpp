#include "network/Room.h"

bool Room::AddPlayer(sf::TcpSocket* socket, const std::string& username)
{
    for (const Player& player : players)
    {
        if (socket == player.GetSocket())
        {
            return false;
        }
    }

    if (players.size() >= kMaxPlayers)
    {
        return false;
    }

    players.emplace_back(socket, username);
    return true;
}

bool Room::RemovePlayer(sf::TcpSocket* socket)
{
    for (auto playerIterator = players.begin(); playerIterator != players.end(); ++playerIterator)
    {
        if (playerIterator->GetSocket() == socket)
        {
            std::cout << "Player removed from room " << id << std::endl;
            players.erase(playerIterator);
            return true;
        }
    }

    return false;
}

bool Room::HasPlayer(sf::TcpSocket* socket) const
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

int Room::GetPlayerIndex(sf::TcpSocket* socket) const
{
    for (std::size_t index = 0; index < players.size(); ++index)
    {
        if (players[index].GetSocket() == socket)
        {
            return static_cast<int>(index);
        }
    }

    return -1;
}
