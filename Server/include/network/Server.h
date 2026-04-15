#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include "core/Constants.h"
#include <iostream>
#include "PacketTypes.h"
#include "database/DatabaseManager.h"
#include "Room.h"
#include "game/GameSession.h"

class Server
{
public:

	Server(){};
	int run();

private:
	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::vector<sf::TcpSocket*> clients;

	DatabaseManager databaseManager;

	std::vector<Room> _rooms;
	std::vector<GameSession> _sessions;

	bool InitializeListener();
	void HandleNewConnection();

	void HandleClientMessages();
	void HandleHandshake(sf::TcpSocket& client, sf::Packet& packet);
	void HandleLogin(sf::TcpSocket& client, sf::Packet packet);
	void HandleRegister(sf::TcpSocket& client, sf::Packet packet);
	void HandleGetRanking(sf::TcpSocket& client, sf::Packet packet);


	bool SendPacket(sf::TcpSocket& socket,sf::Packet& packet, const std::string& context);


	void RemoveClient(std::size_t index);

	void CreateRoom(sf::TcpSocket* client, const std::string& username, std::string roomId);
	void JoinRoom(sf::TcpSocket* client, std::string roomId, std::string& username);
	
	void Shutdown();
};