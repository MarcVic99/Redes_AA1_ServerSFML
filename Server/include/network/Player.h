#pragma once
#include "SFML/Network.hpp"

class Player {

public:

	sf::TcpSocket* socket;
	std::string username;

	Player() : socket(nullptr), username("") {}
	Player(sf::TcpSocket* socket, const std::string& username) : socket(socket), username(username)  {}
	

	sf::TcpSocket* GetSocket() const { return socket; }
	const std::string& GetUsername() const { return username; }
};