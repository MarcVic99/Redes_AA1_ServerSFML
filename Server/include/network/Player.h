#pragma once
#include "SFML/Network.hpp"

class Player {
	sf::TcpSocket* socket;
	std::string username;
public:
	Player(sf::TcpSocket* socket, const std::string& username) : socket(socket), username(username) {}
	sf::TcpSocket* GetSocket() const { return socket; }
	const std::string& GetUsername() const { return username; }
};