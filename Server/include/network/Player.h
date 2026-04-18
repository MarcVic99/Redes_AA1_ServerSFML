#pragma once
#include "SFML/Network.hpp"

enum PlayerColor
{
	Rojo,		//0
	Naranja,	//1
	Azul,		//2
	Verde		//3
};

class Player {


public:

	sf::TcpSocket* socket;
	std::string username;
	PlayerColor color;
	int id;

	Player() : id(-1), socket(nullptr), username(""), color(PlayerColor::Rojo) {}
	Player(sf::TcpSocket* socket, const std::string& username) : socket(socket), username(username)  {}
	

	//getters
	sf::TcpSocket* GetSocket() const { return socket; }
	const std::string& GetUsername() const { return username; }
	PlayerColor GetPlayerColor() const { return color;  }
	int GetPlayerId() const { return id; }

	//setters
	void SetPlayerColor(PlayerColor c) { color = c; }
};