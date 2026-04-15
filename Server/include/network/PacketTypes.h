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
    REGISTER_ERROR,
    GET_RANKING,
    RECEIVE_RANKING,

    CREATE_ROOM,
    CREATE_ROOM_OK,
    CREATE_ROOM_ERROR,
    JOIN_ROOM,
    JOIN_ROOM_OK,
    JOIN_ROOM_ERROR,
    ROOM_WAITING,
    ROOM_READY,
    START_GAME
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