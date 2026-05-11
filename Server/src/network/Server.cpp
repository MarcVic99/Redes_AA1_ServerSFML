#include "network/Server.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>

int Server::Run()
{
    if (!InitializeListener())
    {
        std::cout << "Error listener" << std::endl;
        return -1;
    }

    if (!databaseManager.Connect())
    {
        std::cout << "Error conectando con DB" << std::endl;
        return -1;
    }

    if (sodium_init() < 0)
    {
        std::cout << "Error inicializando libsodium" << std::endl;
        return -1;
    }

    const sf::Time selectorWaitTime = sf::milliseconds(kSelectorWaitTimeMilliseconds);

    while (true)
    {
        if (!selector.wait(selectorWaitTime))
        {
            continue;
        }

        if (selector.isReady(listener))
        {
            HandleNewConnection();
        }

        HandleClientMessages();
    }

    databaseManager.Disconnect();
    Shutdown();
    return 0;
}

bool Server::InitializeListener()
{
    if (listener.listen(kListenerPort, sf::IpAddress::Any) != sf::Socket::Status::Done)
    {
        std::cout << "Error al intentar escuchar en el puerto " << kListenerPort << std::endl;
        return false;
    }

    selector.add(listener);
    std::cout << "Servidor bootstrap escuchando en puerto " << kListenerPort << std::endl;
    return true;
}

void Server::HandleNewConnection()
{
    sf::TcpSocket* newClient = new sf::TcpSocket();

    if (listener.accept(*newClient) == sf::Socket::Status::Done)
    {
        selector.add(*newClient);
        clients.push_back(newClient);

        std::cout << "Nueva conexion establecida" << std::endl;
        return;
    }

    delete newClient;
}

void Server::HandleClientMessages()
{
    for (std::size_t index = 0; index < clients.size(); ++index)
    {
        sf::TcpSocket* client = clients[index];

        if (!selector.isReady(*client))
        {
            continue;
        }

        sf::Packet packet;
        const sf::Socket::Status status = client->receive(packet);

        if (status == sf::Socket::Status::Done)
        {
            tipoPaquete tipo;
            packet >> tipo;

            switch (tipo)
            {
            case tipoPaquete::HANDSHAKE:
                HandleHandshake(*client, packet);
                break;

            case tipoPaquete::LOGIN:
                HandleLogin(*client, packet);
                break;

            case tipoPaquete::REGISTER:
                HandleRegister(*client, packet);
                break;

            case tipoPaquete::GET_RANKING:
                HandleGetRanking(*client, packet);
                break;

            case tipoPaquete::PEER_READY:
                HandlePeerReady(*client, packet);
                break;

            case tipoPaquete::CREATE_ROOM:
            {
                std::string roomId;
                packet >> roomId;
                CreateRoom(client, roomId);
                break;
            }

            case tipoPaquete::JOIN_ROOM:
            {
                std::string roomId;
                packet >> roomId;
                JoinRoom(client, roomId);
                break;
            }

            case tipoPaquete::REPORT_MATCH_RESULT:
                HandleMatchResult(*client, packet);
                break;

            default:
                std::cout << "Paquete no gestionado por el bootstrap server" << std::endl;
                break;
            }
        }
        else if (status == sf::Socket::Status::Disconnected)
        {
            std::cout << "Cliente desconectado" << std::endl;
            RemoveClientFromRooms(client);
            RemoveClient(index);
            --index;
        }
    }
}

void Server::HandleHandshake(sf::TcpSocket& client, sf::Packet& packet)
{
    static const std::string kHelloServerMessage = "HELLO_SERVER";
    static const std::string kHelloClientMessage = "HELLO_CLIENT";
    static const std::string kHelloErrorMessage = "HELLO_ERROR";

    std::string message;
    packet >> message;

    std::cout << "Handshake recibido: " << message << std::endl;

    sf::Packet response;

    if (message == kHelloServerMessage)
    {
        response << tipoPaquete::HANDSHAKE_OK << kHelloClientMessage;
    }
    else
    {
        response << tipoPaquete::HANDSHAKE_ERROR << kHelloErrorMessage;
    }

    SendPacket(client, response, "handshake");
}

void Server::HandleLogin(sf::TcpSocket& client, sf::Packet& packet)
{
    static const std::string kClientVerifiedMessage = "CLIENT_VERIFIED";
    static const std::string kLoginErrorMessage = "LOGIN_ERROR";

    std::string username;
    std::string password;
    packet >> username >> password;

    std::cout << "Login recibido de usuario: " << username << std::endl;

    sf::Packet response;

    if (databaseManager.ValidateLogin(username, password))
    {
        const PlayerData playerData = databaseManager.GetPlayerbyName(username);

        authenticatedClients[&client] =
        {
            playerData.id,
            playerData.user,
            0
        };

        response << tipoPaquete::LOGIN_OK << kClientVerifiedMessage;
    }
    else
    {
        response << tipoPaquete::LOGIN_ERROR << kLoginErrorMessage;
    }

    if (!SendPacket(client, response, "login"))
    {
        return;
    }

    if (IsClientAuthenticated(&client))
    {
        SendPlayerInfo(client, username);
    }
}

void Server::HandleRegister(sf::TcpSocket& client, sf::Packet& packet)
{
    static const std::string kClientCreatedMessage = "CLIENT_CREATED";
    static const std::string kRegisterErrorMessage = "REGISTER_ERROR";

    std::string username;
    std::string password;
    packet >> username >> password;

    std::cout << "Registro recibido de usuario: " << username << std::endl;

    sf::Packet response;

    if (databaseManager.RegisterUser(username, password))
    {
        const PlayerData playerData = databaseManager.GetPlayerbyName(username);

        authenticatedClients[&client] =
        {
            playerData.id,
            playerData.user,
            0
        };

        response << tipoPaquete::REGISTER_OK << kClientCreatedMessage;
    }
    else
    {
        response << tipoPaquete::REGISTER_ERROR << kRegisterErrorMessage;
    }

    if (!SendPacket(client, response, "register"))
    {
        return;
    }

    if (IsClientAuthenticated(&client))
    {
        SendPlayerInfo(client, username);
    }
}

void Server::HandleGetRanking(sf::TcpSocket& client, sf::Packet& packet)
{
    int userId = 0;
    packet >> userId;

    std::cout << "Cliente pide acceso al ranking. Cliente ID: " << userId << std::endl;

    const std::vector<PlayerData> topPlayers = databaseManager.GetTopPlayers();

    sf::Packet response;
    response << tipoPaquete::RECEIVE_RANKING;
    response << static_cast<std::int32_t>(topPlayers.size());

    for (std::size_t index = 0; index < topPlayers.size(); ++index)
    {
        const PlayerData& playerData = topPlayers[index];

        response << static_cast<std::int32_t>(index + 1);
        response << playerData.user;
        response << static_cast<std::int32_t>(playerData.puntuacion_total);
        response << static_cast<std::int32_t>(playerData.victorias);
        response << static_cast<std::int32_t>(playerData.derrotas);
    }

    const PlayerData currentUserData = databaseManager.GetPlayerById(userId);
    const int userRank = databaseManager.GetPlayerRank(userId);

    response << userRank;
    response << currentUserData.user;
    response << currentUserData.puntuacion_total;
    response << currentUserData.victorias;
    response << currentUserData.derrotas;

    SendPacket(client, response, "get_ranking");
}

void Server::HandlePeerReady(sf::TcpSocket& client, sf::Packet& packet)
{
    std::uint16_t peerPort = 0;
    packet >> peerPort;

    if (!IsClientAuthenticated(&client))
    {
        std::cout << "PEER_READY rechazado: cliente no autenticado" << std::endl;
        return;
    }

    if (peerPort == 0)
    {
        std::cout << "PEER_READY rechazado: puerto P2P invalido" << std::endl;
        return;
    }

    authenticatedClients[&client].peerPort = peerPort;

    std::cout << "Cliente " << authenticatedClients[&client].username
        << " escuchara P2P en puerto " << peerPort << std::endl;

    sf::Packet response;
    response << tipoPaquete::PEER_READY_OK;
    SendPacket(client, response, "peer_ready_ok");
}

void Server::HandleMatchResult(sf::TcpSocket& client, sf::Packet& packet)
{
    if (!IsClientAuthenticated(&client))
    {
        std::cout << "Resultado rechazado: cliente no autenticado" << std::endl;
        return;
    }

    std::string roomId;
    std::int32_t totalPlayers = 0;

    packet >> roomId >> totalPlayers;

    MatchReportData report;
    report.reporterUserId = authenticatedClients[&client].userId;

    for (std::int32_t index = 0; index < totalPlayers; ++index)
    {
        std::string username;
        packet >> username;
        report.orderedPlayers.push_back(username);
    }

    PendingMatchResult& pendingResult = pendingResults[roomId];

    auto existingReport = std::find_if(
        pendingResult.reports.begin(),
        pendingResult.reports.end(),
        [&](const MatchReportData& currentReport)
        {
            return currentReport.reporterUserId == report.reporterUserId;
        });

    if (existingReport != pendingResult.reports.end())
    {
        *existingReport = report;
    }
    else
    {
        pendingResult.reports.push_back(report);
    }

    std::cout << "Resultado recibido para room " << roomId
        << " desde usuario " << authenticatedClients[&client].username
        << std::endl;

    if (!pendingResult.rankingApplied)
    {
        for (std::size_t first = 0; first < pendingResult.reports.size(); ++first)
        {
            for (std::size_t second = first + 1; second < pendingResult.reports.size(); ++second)
            {
                if (pendingResult.reports[first].orderedPlayers ==
                    pendingResult.reports[second].orderedPlayers)
                {
                    ApplyValidatedResult(pendingResult.reports[first].orderedPlayers);
                    pendingResult.rankingApplied = true;

                    std::cout << "Resultado validado por pares para room "
                        << roomId << std::endl;
                    break;
                }
            }

            if (pendingResult.rankingApplied)
            {
                break;
            }
        }
    }

    sf::Packet response;

    if (pendingResult.rankingApplied)
    {
        response << tipoPaquete::REPORT_MATCH_RESULT_OK;
        SendPacket(client, response, "match_result_ok");
    }
    else
    {
        response << tipoPaquete::REPORT_MATCH_RESULT_ERROR;
        SendPacket(client, response, "match_result_pending");
    }
}

bool Server::SendPacket(sf::TcpSocket& socket, sf::Packet& packet, const std::string& context)
{
    const sf::Socket::Status status = socket.send(packet);

    if (status != sf::Socket::Status::Done)
    {
        std::cerr << "Error al enviar " << context
            << " status=" << static_cast<int>(status) << std::endl;
        return false;
    }

    return true;
}

void Server::RemoveClient(std::size_t index)
{
    sf::TcpSocket* client = clients[index];

    authenticatedClients.erase(client);
    selector.remove(*client);
    delete client;

    clients.erase(clients.begin() + static_cast<std::ptrdiff_t>(index));
}

void Server::RemoveClientFromRooms(sf::TcpSocket* client)
{
    for (std::size_t roomIndex = 0; roomIndex < rooms.size();)
    {
        Room& room = rooms[roomIndex];
        const std::string roomId = room.GetId();

        room.RemovePlayer(client);

        if (room.GetPlayers().empty())
        {
            std::cout << "Sala " << roomId << " eliminada porque se ha quedado vacia" << std::endl;
            rooms.erase(rooms.begin() + static_cast<std::ptrdiff_t>(roomIndex));
            continue;
        }

        ++roomIndex;
    }
}

void Server::CreateRoom(sf::TcpSocket* client, const std::string& roomId)
{
    if (!IsClientAuthenticated(client))
    {
        std::cout << "CREATE_ROOM rechazado: cliente no autenticado" << std::endl;
        return;
    }

    if (GetPeerPort(client) == 0)
    {
        std::cout << "CREATE_ROOM rechazado: el cliente aun no ha enviado su puerto P2P" << std::endl;
        return;
    }

    for (const Room& room : rooms)
    {
        if (room.GetId() == roomId)
        {
            sf::Packet packet;
            packet << tipoPaquete::CREATE_ROOM_ERROR;
            SendPacket(*client, packet, "create_room_error");

            std::cout << "Room id " << roomId << " already exists" << std::endl;
            return;
        }
    }

    Room newRoom;
    newRoom.SetId(roomId);
    newRoom.AddPlayer(client, GetAuthenticatedUsername(client));
    rooms.push_back(newRoom);

    sf::Packet packet;
    packet << tipoPaquete::CREATE_ROOM_OK << roomId;
    SendPacket(*client, packet, "create_room_ok");

    std::cout << "Created room id: " << roomId << std::endl;
}

void Server::JoinRoom(sf::TcpSocket* client, const std::string& roomId)
{
    if (!IsClientAuthenticated(client))
    {
        std::cout << "JOIN_ROOM rechazado: cliente no autenticado" << std::endl;
        return;
    }

    if (GetPeerPort(client) == 0)
    {
        std::cout << "JOIN_ROOM rechazado: el cliente aun no ha enviado su puerto P2P" << std::endl;
        return;
    }

    for (std::size_t roomIndex = 0; roomIndex < rooms.size(); ++roomIndex)
    {
        Room& room = rooms[roomIndex];

        if (room.GetId() != roomId)
        {
            continue;
        }

        if (room.IsFull())
        {
            sf::Packet packet;
            packet << tipoPaquete::JOIN_ROOM_ERROR;
            SendPacket(*client, packet, "join_room_error");
            return;
        }

        if (room.HasPlayer(client))
        {
            sf::Packet packet;
            packet << tipoPaquete::JOIN_ROOM_ERROR;
            SendPacket(*client, packet, "join_room_error");
            return;
        }

        room.AddPlayer(client, GetAuthenticatedUsername(client));

        sf::Packet packet;
        packet << tipoPaquete::JOIN_ROOM_OK << roomId;
        SendPacket(*client, packet, "join_room_ok");

        std::cout << "Joined room " << roomId
            << ", players now: " << room.GetPlayers().size() << std::endl;

        if (room.IsFull())
        {
            std::cout << "Room " << roomId << " is full. Preparing P2P match..." << std::endl;
            SendMatchReady(room);
            rooms.erase(rooms.begin() + static_cast<std::ptrdiff_t>(roomIndex));
        }

        return;
    }

    sf::Packet packet;
    packet << tipoPaquete::JOIN_ROOM_ERROR;
    SendPacket(*client, packet, "join_room_error");

    std::cout << "Room id " << roomId << " doesn't exist" << std::endl;
}

void Server::SendMatchReady(Room& room)
{
    const std::vector<Player>& roomPlayers = room.GetPlayers();

    if (roomPlayers.empty())
    {
        std::cout << "No se puede preparar START_GAME: la sala esta vacia" << std::endl;
        return;
    }

    struct PreparedPlayerData
    {
        std::string username;
        std::int32_t userId = -1;
        std::int32_t rankingPoints = 0;
        std::string ip;
        std::uint16_t peerPort = 0;
        std::int32_t playerIndex = -1;
    };

    std::vector<PreparedPlayerData> preparedPlayers;
    preparedPlayers.reserve(roomPlayers.size());

    for (std::size_t index = 0; index < roomPlayers.size(); ++index)
    {
        const Player& currentPlayer = roomPlayers[index];
        sf::TcpSocket* currentSocket = currentPlayer.GetSocket();

        if (currentSocket == nullptr)
        {
            std::cout << "No se puede preparar START_GAME: socket nulo" << std::endl;
            return;
        }

        const std::optional<sf::IpAddress> remoteAddressOptional =
            currentSocket->getRemoteAddress();

        if (!remoteAddressOptional.has_value())
        {
            std::cout << "No se puede preparar START_GAME: no se ha podido obtener la IP de "
                << currentPlayer.GetUsername() << std::endl;
            return;
        }

        const std::uint16_t peerPort = GetPeerPort(currentSocket);
        if (peerPort == 0)
        {
            std::cout << "No se puede preparar START_GAME: puerto P2P invalido de "
                << currentPlayer.GetUsername() << std::endl;
            return;
        }

        const PlayerData databaseData =
            databaseManager.GetPlayerbyName(currentPlayer.GetUsername());

        if (databaseData.user.empty())
        {
            std::cout << "No se puede preparar START_GAME: jugador no encontrado en DB: "
                << currentPlayer.GetUsername() << std::endl;
            return;
        }

        PreparedPlayerData prepared;
        prepared.username = currentPlayer.GetUsername();
        prepared.userId = static_cast<std::int32_t>(databaseData.id);
        prepared.rankingPoints = static_cast<std::int32_t>(databaseData.puntuacion_total);
        prepared.ip = remoteAddressOptional->toString();
        prepared.peerPort = peerPort;
        prepared.playerIndex = static_cast<std::int32_t>(index);

        preparedPlayers.push_back(prepared);
    }

    for (const Player& targetPlayer : roomPlayers)
    {
        sf::TcpSocket* targetSocket = targetPlayer.GetSocket();

        if (targetSocket == nullptr)
        {
            continue;
        }

        sf::Packet startPacket;
        startPacket << tipoPaquete::START_GAME;
        startPacket << room.GetId();
        startPacket << static_cast<std::int32_t>(preparedPlayers.size());

        for (const PreparedPlayerData& prepared : preparedPlayers)
        {
            startPacket << prepared.username;
            startPacket << prepared.userId;
            startPacket << prepared.rankingPoints;
            startPacket << prepared.ip;
            startPacket << prepared.peerPort;
            startPacket << prepared.playerIndex;
        }

        SendPacket(*targetSocket, startPacket, "start_game");
    }

    std::cout << "START_GAME enviado para la room "
        << room.GetId()
        << " con " << preparedPlayers.size()
        << " jugadores" << std::endl;
}

void Server::SendPlayerInfo(sf::TcpSocket& client, const std::string& username)
{
    const PlayerData playerData = databaseManager.GetPlayerbyName(username);

    if (playerData.user.empty())
    {
        std::cout << "Error: user not found in DB" << std::endl;
        return;
    }

    sf::Packet packet;
    packet << tipoPaquete::USER_INFO
        << static_cast<std::int32_t>(playerData.id)
        << playerData.user;

    std::cout << "Sending player info to client: ID=" << playerData.id
        << " Username='" << playerData.user << "'"
        << std::endl;

    SendPacket(client, packet, "user_info");
}

void Server::ApplyValidatedResult(const std::vector<std::string>& orderedPlayers)
{
    for (std::size_t index = 0; index < orderedPlayers.size(); ++index)
    {
        const bool isWinner = index < kWinnerPoints.size();
        const int pointsDelta = isWinner ? kWinnerPoints[index] : kLoserPoints;

        databaseManager.UpdatePlayerStats(
            orderedPlayers[index],
            pointsDelta,
            isWinner,
            !isWinner);
    }
}

bool Server::IsClientAuthenticated(sf::TcpSocket* client) const
{
    return authenticatedClients.find(client) != authenticatedClients.end();
}

std::string Server::GetAuthenticatedUsername(sf::TcpSocket* client) const
{
    const auto it = authenticatedClients.find(client);

    if (it == authenticatedClients.end())
    {
        return "";
    }

    return it->second.username;
}

std::uint16_t Server::GetPeerPort(sf::TcpSocket* client) const
{
    const auto it = authenticatedClients.find(client);

    if (it == authenticatedClients.end())
    {
        return 0;
    }

    return it->second.peerPort;
}

void Server::Shutdown()
{
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }

    clients.clear();
    authenticatedClients.clear();
    pendingResults.clear();
    rooms.clear();
}