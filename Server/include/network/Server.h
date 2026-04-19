#pragma once

#include <SFML/Network.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "PacketTypes.h"
#include "Room.h"
#include "core/Constants.h"
#include "database/DatabaseManager.h"
#include "game/GameSession.h"

class Server
{
public:
    Server() = default;

    /** Starts the server loop and processes incoming requests. */
    int Run();

private:
    /** Initializes the TCP listener used by the server. */
    bool InitializeListener();

    /** Accepts a new client connection. */
    void HandleNewConnection();

    /** Processes all pending messages received from connected clients. */
    void HandleClientMessages();

    /** Validates the handshake packet and sends the corresponding response. */
    void HandleHandshake(sf::TcpSocket& client, sf::Packet& packet);

    /** Handles a client login request. */
    void HandleLogin(sf::TcpSocket& client, sf::Packet& packet);

    /** Handles a client registration request. */
    void HandleRegister(sf::TcpSocket& client, sf::Packet& packet);

    /** Sends ranking information to the requesting client. */
    void HandleGetRanking(sf::TcpSocket& client, sf::Packet& packet);

    /** Sends a packet and reports an error when the operation fails. */
    bool SendPacket(sf::TcpSocket* socket, sf::Packet& packet, const std::string& context);

    /** Removes a disconnected client from the server structures. */
    void RemoveClient(std::size_t index);

    /** Creates a new room and places the creator inside it. */
    void CreateRoom(sf::TcpSocket* client, const std::string& username, const std::string& roomId);

    /** Adds a player to an existing room when possible. */
    void JoinRoom(sf::TcpSocket* client, const std::string& roomId, const std::string& username);

    /** Sends the player data required by the client after authentication. */
    void SendPlayerInfo(sf::TcpSocket& client, const std::string& username);

    /** Sends the player list of an active session. */
    void SendPlayers(sf::TcpSocket* client, const std::vector<Player>& players);

    /** Finalizes a match and broadcasts the result to all players. */
    void CheckFinish(const std::vector<Player>& players, const std::vector<Player>& winners, const std::vector<Player>& losers, bool isFinished);

    /** Broadcasts a move to the rest of the players in the session. */
    void BroadcastPlayerMove(GameSession* session, sf::TcpSocket* sender, Cell cell, int row, int column);

    /** Broadcasts that a turn has been skipped because of timeout. */
    void BroadcastSkipTurnTimeout(GameSession* session);

    /** Returns the active session associated with a client. */
    GameSession* GetSessionByClient(sf::TcpSocket* client);

    /** Releases the memory owned by the server before exit. */
    void Shutdown();

    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<sf::TcpSocket*> clients;
    DatabaseManager databaseManager;
    std::vector<Room> rooms;
    std::vector<GameSession> sessions;
};
