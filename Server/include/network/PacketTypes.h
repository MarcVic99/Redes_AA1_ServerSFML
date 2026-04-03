#pragma once
#include <SFML/Network.hpp>


enum class tipoPaquete {
    HANDSHAKE,
    HANDSHAKE_OK,
    HANDSHAKE_ERROR,
    LOGIN,
    LOGIN_OK,
    LOGIN_ERROR,
    REGISTER,
    REGISTER_OK,
    REGISTER_ERROR
};

inline sf::Packet& operator>>(sf::Packet& packet, tipoPaquete& tipo)
{
    int temp;
    packet >> temp;
    tipo = static_cast<tipoPaquete>(temp);

    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const tipoPaquete& tipo)
{
    //Convierte enum a int y lo pasa en packet
    return packet << static_cast<int>(tipo);
}