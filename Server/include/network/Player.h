#pragma once

#include <string>

#include <SFML/Network.hpp>

enum PlayerColor
{
    Rojo,
    Naranja,
    Azul,
    Verde
};

class Player
{
public:
    Player();
    Player(sf::TcpSocket* socket, const std::string& username);

    /** Returns the socket associated with the player. */
    sf::TcpSocket* GetSocket() const { return socket; }

    /** Returns the username associated with the player. */
    const std::string& GetUsername() const { return username; }

    /** Returns the color assigned to the player in the match. */
    PlayerColor GetPlayerColor() const { return color; }

    /** Returns the identifier of the player. */
    int GetPlayerId() const { return id; }

    /** Assigns a color to the player. */
    void SetPlayerColor(PlayerColor newColor) { color = newColor; }

    /** Assigns an identifier to the player. */
    void SetPlayerId(int newId) { id = newId; }

private:
    sf::TcpSocket* socket;
    std::string username;
    PlayerColor color;
    int id;
};

inline Player::Player()
    : socket(nullptr), username(""), color(PlayerColor::Rojo), id(-1)
{
}

inline Player::Player(sf::TcpSocket* socketValue, const std::string& usernameValue)
    : socket(socketValue), username(usernameValue), color(PlayerColor::Rojo), id(-1)
{
}
