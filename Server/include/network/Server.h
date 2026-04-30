#pragma once

#include <SFML/Network.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "PacketTypes.h"
#include "Room.h"
#include "core/Constants.h"
#include "database/DatabaseManager.h"

class Server
{
public:
    Server() = default;

    /*
       Bucle principal del bootstrap server.
       Se gestionan login, salas, ranking
       y el arranque de la conexion P2P entre clientes.
     */
    int Run();

private:
    /* Inicializa el listener TCP del servidor. */
    bool InitializeListener();

    /* Acepta una nueva conexion entrante. */
    void HandleNewConnection();

    /* Procesa los paquetes pendientes de todos los clientes conectados. */
    void HandleClientMessages();

    /* Gestion del handshake inicial. */
    void HandleHandshake(sf::TcpSocket& client, sf::Packet& packet);

    /* Gestion del login. */
    void HandleLogin(sf::TcpSocket& client, sf::Packet& packet);

    /* Gestion del registro. */
    void HandleRegister(sf::TcpSocket& client, sf::Packet& packet);

    /* Envia el ranking al cliente que lo pide. */
    void HandleGetRanking(sf::TcpSocket& client, sf::Packet& packet);

    /*
      El cliente avisa de en que puerto escuchara conexiones P2P.
      Esto lo necesitamos para poder pasarle al resto la info del host.
     */
    void HandlePeerReady(sf::TcpSocket& client, sf::Packet& packet);

    /*
      Recibe el resultado final de una partida.
      El ranking solo se actualiza cuando llegan al menos dos reportes identicos.
     */
    void HandleMatchResult(sf::TcpSocket& client, sf::Packet& packet);

    /* Envia un paquete y devuelve false si hubo error. */
    bool SendPacket(sf::TcpSocket* socket, sf::Packet& packet, const std::string& context);

    /* Elimina un cliente de las estructuras internas. */
    void RemoveClient(std::size_t index);

    /* Limpia al cliente de las salas en las que estuviera. */
    void RemoveClientFromRooms(sf::TcpSocket* client);

    /* Borra una sala si se ha quedado vacia. */
    void RemoveRoomIfEmpty(const std::string& roomId);

    /* Crea una sala nueva. */
    void CreateRoom(sf::TcpSocket* client, const std::string& roomId);

    /* Une a un jugador a una sala existente. */
    void JoinRoom(sf::TcpSocket* client, const std::string& roomId);

    /*
      Cuando la sala esta completa, enviamos a todos la informacion
      necesaria para que se conecten P2P con el host.
     */
    void SendMatchReady(Room& room);

    /* Envia al cliente su id y su username despues de auth. */
    void SendPlayerInfo(sf::TcpSocket& client, const std::string& username);

    /* Aplica al ranking un resultado ya validado por pares. */
    void ApplyValidatedResult(const std::vector<std::string>& orderedPlayers);

    /* Comprueba si el cliente ya esta autenticado. */
    bool IsClientAuthenticated(sf::TcpSocket* client) const;

    /* Devuelve el username autenticado de ese socket. */
    std::string GetAuthenticatedUsername(sf::TcpSocket* client) const;

    /* Devuelve el puerto P2P registrado por ese cliente. */
    std::uint16_t GetPeerPort(sf::TcpSocket* client) const;

    /* Libera memoria antes de cerrar el servidor. */
    void Shutdown();

private:
    struct ClientSessionData
    {
        int userId = -1;
        std::string username;
        bool authenticated = false;
        std::uint16_t peerPort = 0;
    };

    struct MatchReportData
    {
        int reporterUserId = -1;
        std::vector<std::string> orderedPlayers;
    };

    struct PendingMatchResult
    {
        std::vector<MatchReportData> reports;
        bool rankingApplied = false;
    };

    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<sf::TcpSocket*> clients;
    DatabaseManager databaseManager;
    std::vector<Room> rooms;

    // Guardamos el estado de sesion de cada socket para no depender
    // solo de lo que mande el cliente en cada paquete.
    std::unordered_map<sf::TcpSocket*, ClientSessionData> authenticatedClients;

    // Resultados pendientes de validar por pares.
    std::unordered_map<std::string, PendingMatchResult> pendingResults;
};