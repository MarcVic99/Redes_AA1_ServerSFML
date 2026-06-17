#pragma once

#include <SFML/Network.hpp>

#include <cstddef>
#include <cstdint>
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
    int Run();

private:
    bool InitializeListener();
    void HandleNewConnection();
    void HandleClientMessages();

    void HandleHandshake(sf::TcpSocket& client, sf::Packet& packet);
    void HandleLogin(sf::TcpSocket& client, sf::Packet& packet);
    void HandleRegister(sf::TcpSocket& client, sf::Packet& packet);
    void HandleGetRanking(sf::TcpSocket& client, sf::Packet& packet);
    void HandlePeerReady(sf::TcpSocket& client, sf::Packet& packet);
    void HandleMatchResult(sf::TcpSocket& client, sf::Packet& packet);

    bool SendPacket(sf::TcpSocket& socket, sf::Packet& packet, const std::string& context);

    void RemoveClient(std::size_t index);
    void RemoveClientFromRooms(sf::TcpSocket* client);

    void CreateRoom(sf::TcpSocket* client, const std::string& roomId);
    void JoinRoom(sf::TcpSocket* client, const std::string& roomId);
    void SendMatchReady(Room& room);

    void SendPlayerInfo(sf::TcpSocket& client, const std::string& username);
    void ApplyValidatedResult(const std::vector<std::string>& orderedPlayers);

    bool IsClientAuthenticated(sf::TcpSocket* client) const;
    std::string GetAuthenticatedUsername(sf::TcpSocket* client) const;
    std::uint16_t GetPeerPort(sf::TcpSocket* client) const;

    void Shutdown();

private:
    struct ClientSessionData
    {
        int userId = -1;
        std::string username;
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

    std::unordered_map<sf::TcpSocket*, ClientSessionData> authenticatedClients;
    std::unordered_map<std::string, PendingMatchResult> pendingResults;
};