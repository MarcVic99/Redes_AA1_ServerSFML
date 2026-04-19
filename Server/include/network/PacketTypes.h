#pragma once

#include <cstdint>

#include <SFML/Network.hpp>

enum class tipoPaquete
{
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
    START_GAME,
    PLAYERS_GAME_REQUEST,
    PLAYERS_GAME_RESPONSE,
    USER_INFO,
    DELETE_BOARD,
    PLAYER_MOVE,
    BROADCAST_PLAYER_MOVE,
    SKIP_TURN,
    GAME_FINISHED
};

inline sf::Packet& operator>>(sf::Packet& packet, tipoPaquete& tipo)
{
    std::int32_t rawValue = 0;
    packet >> rawValue;
    tipo = static_cast<tipoPaquete>(rawValue);

    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const tipoPaquete& tipo)
{
    return packet << static_cast<std::int32_t>(tipo);
}
