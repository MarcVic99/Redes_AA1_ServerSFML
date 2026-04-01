#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include "../core/Constants.h"
#include <iostream>

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
	void removeClient(std::size_t index);
	
	void shutdown();

};