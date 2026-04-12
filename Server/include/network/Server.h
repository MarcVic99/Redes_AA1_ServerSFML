#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include "core/Constants.h"
#include <iostream>
#include "PacketTypes.h"
#include "database/DatabaseManager.h"
#include "Room.h"

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
	std::string _nextRoomId = "1";

	bool initializeListener();
	void handleNewConnection();

	void handleClientMessages();
	void handleHandshake(sf::TcpSocket& client, sf::Packet& packet);
	void handleLogin(sf::TcpSocket& client, sf::Packet packet);
	void handleRegister(sf::TcpSocket& client, sf::Packet packet);
	void handleGetRanking(sf::TcpSocket& client, sf::Packet packet);


	bool sendPacket(sf::TcpSocket& socket,sf::Packet& packet, const std::string& context);


	void removeClient(std::size_t index);

	void CreateRoom(sf::TcpSocket* client, const std::string& username, std::string roomId);
	void JoinRoom(sf::TcpSocket* client, std::string roomId, std::string& username);
	
	void shutdown();

};