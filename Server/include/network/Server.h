#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include "core/Constants.h"
#include <iostream>
#include "PacketTypes.h"

class Server
{
public:

	Server(){};
	int run();

private:
	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::vector<sf::TcpSocket*> clients;

	bool initializeListener();
	void handleNewConnection();

	void handleClientMessages();
	void handleHandshake(sf::TcpSocket& client, sf::Packet& packet);
	void handleLogin(sf::TcpSocket& client, sf::Packet packet);
	void handleRegister(sf::TcpSocket& client, sf::Packet packet);


	bool sendPacket(sf::TcpSocket& socket,sf::Packet& packet, const std::string& context);


	void removeClient(std::size_t index);
	
	void shutdown();

};