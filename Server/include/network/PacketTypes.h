#pragma once

#include <cstdint>

#include <SFML/Network.hpp>

enum class tipoPaquete
{
    // =========================
    // Handshake / autenticacion
    // =========================
    HANDSHAKE,
    HANDSHAKE_OK,
    HANDSHAKE_ERROR,

    LOGIN,
    LOGIN_OK,
    LOGIN_ERROR,

    REGISTER,
    REGISTER_OK,
    REGISTER_ERROR,

    USER_INFO,

    // =========================
    // Ranking
    // =========================
    GET_RANKING,
    RECEIVE_RANKING,

    // =========================
    // Lobby / bootstrap
    // =========================
    CREATE_ROOM,
    CREATE_ROOM_OK,
    CREATE_ROOM_ERROR,

    JOIN_ROOM,
    JOIN_ROOM_OK,
    JOIN_ROOM_ERROR,

    /*
        El cliente avisa al bootstrap del puerto en el que aceptara
        conexiones P2P. Esto nos hace falta antes de montar la partida.
    */
    PEER_READY,
    PEER_READY_OK,

    /*
        Aunque se llama START_GAME para mantener compatibilidad con la base,
        ahora este paquete no significa que el servidor vaya a arbitrar la partida.
        Significa que la sala ya esta lista y se envia la informacion necesaria
        para que los clientes se conecten entre ellos en P2P.
    */
    START_GAME,

    // =========================
    // Gameplay P2P
    // =========================
    /*
        Estos paquetes ya no pertenecen al bootstrap server.
        Se usan durante la partida entre el host y el resto de peers.
    */
    PLAYERS_GAME_REQUEST,
    PLAYERS_GAME_RESPONSE,

    PLAYER_MOVE,
    BROADCAST_PLAYER_MOVE,
    SKIP_TURN,
<<<<<<< Updated upstream
    GAME_FINISHED
=======
    GAME_FINISHED,

    // Legacy / pendiente de decidir si se elimina
    DELETE_BOARD,

    // =========================
    // Resultado final de partida
    // =========================
    /*
        Al terminar la partida, los clientes vuelven a hablar con el bootstrap
        y le envian el resultado. El servidor solo actualiza ranking si recibe
        al menos dos resultados identicos.
    */
    REPORT_MATCH_RESULT,
    REPORT_MATCH_RESULT_OK,
    REPORT_MATCH_RESULT_ERROR
>>>>>>> Stashed changes
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