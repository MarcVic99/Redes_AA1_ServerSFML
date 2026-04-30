#include "network/Server.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
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
        if (selector.wait(selectorWaitTime))
        {
            if (selector.isReady(listener))
            {
                HandleNewConnection();
            }

            HandleClientMessages();
        }
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
        newClient->setBlocking(false);
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
                std::string ignoredUsername;
                packet >> roomId >> ignoredUsername;

                CreateRoom(client, roomId);
                break;
            }

            case tipoPaquete::JOIN_ROOM:
            {
                std::string roomId;
                std::string ignoredUsername;
                packet >> roomId >> ignoredUsername;

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
            authenticatedClients.erase(client);

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

    SendPacket(&client, response, "handshake");
}

void Server::HandleLogin(sf::TcpSocket& client, sf::Packet& packet)
{
    static const std::string kClientVerifiedMessage = "CLIENT_VERIFIED";
    static const std::string kLoginErrorMessage = "LOGIN_ERROR";

    std::string username;
    std::string password;

    packet >> username >> password;

    // Ya no mostramos la password por consola. Para debug esta bien saber
    // que ha llegado algo, pero no hace falta imprimir datos sensibles.
    std::cout << "Login recibido de usuario: " << username << std::endl;

    sf::Packet response;
    if (databaseManager.ValidateLogin(username, password))
    {
        const PlayerData playerData = databaseManager.GetPlayerbyName(username);

        authenticatedClients[&client].userId = playerData.id;
        authenticatedClients[&client].username = playerData.user;
        authenticatedClients[&client].authenticated = true;

        response << tipoPaquete::LOGIN_OK << kClientVerifiedMessage;
    }
    else
    {
        response << tipoPaquete::LOGIN_ERROR << kLoginErrorMessage;
    }

    if (!SendPacket(&client, response, "login"))
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

        authenticatedClients[&client].userId = playerData.id;
        authenticatedClients[&client].username = playerData.user;
        authenticatedClients[&client].authenticated = true;

        response << tipoPaquete::REGISTER_OK << kClientCreatedMessage;
    }
    else
    {
        response << tipoPaquete::REGISTER_ERROR << kRegisterErrorMessage;
    }

    if (!SendPacket(&client, response, "register"))
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

    // Enviamos el top que tengamos en DB. Si en el cliente luego quereis
    // cortar a top 10 exacto, se puede hacer alli, pero la idea es que salga
    // ya preparado desde servidor.
    for (std::size_t index = 0; index < topPlayers.size(); ++index)
    {
        const PlayerData& playerData = topPlayers[index];

        response << static_cast<std::int32_t>(index + 1);
        response << playerData.user;
        response << static_cast<std::int32_t>(playerData.puntuacion_total);
        response << static_cast<std::int32_t>(playerData.victorias);
        response << static_cast<std::int32_t>(playerData.derrotas);

        std::cout << "Ranking: " << playerData.user
            << " Puntuacion: " << playerData.puntuacion_total
            << " Victorias: " << playerData.victorias
            << " Derrotas: " << playerData.derrotas
            << std::endl;
    }

    const PlayerData currentUserData = databaseManager.GetPlayerById(userId);
    const int userRank = databaseManager.GetPlayerRank(userId);

    response << userRank;
    response << currentUserData.user;
    response << currentUserData.puntuacion_total;
    response << currentUserData.victorias;
    response << currentUserData.derrotas;

    SendPacket(&client, response, "get_ranking");
}

void Server::HandlePeerReady(sf::TcpSocket& client, sf::Packet& packet)
{
    int peerPort = 0;
    packet >> peerPort;

    if (!IsClientAuthenticated(&client))
    {
        std::cout << "PEER_READY rechazado: cliente no autenticado" << std::endl;
        return;
    }

    authenticatedClients[&client].peerPort = peerPort;

    std::cout << "Cliente " << authenticatedClients[&client].username
        << " escuchara P2P en puerto " << peerPort << std::endl;

    sf::Packet response;
    response << tipoPaquete::PEER_READY_OK;
    SendPacket(&client, response, "peer_ready_ok");
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

    // Si este jugador ya habia reportado, no dejamos duplicados.
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
                if (pendingResult.reports[first].orderedPlayers == pendingResult.reports[second].orderedPlayers)
                {
                    ApplyValidatedResult(pendingResult.reports[first].orderedPlayers);
                    pendingResult.rankingApplied = true;

                    std::cout << "Resultado validado por pares para room " << roomId << std::endl;
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
        SendPacket(&client, response, "match_result_ok");
    }
    else
    {
        response << tipoPaquete::REPORT_MATCH_RESULT_ERROR;
        SendPacket(&client, response, "match_result_pending");
    }
}

bool Server::SendPacket(sf::TcpSocket* socket, sf::Packet& packet, const std::string& context)
{
    if (socket == nullptr)
    {
        std::cerr << "Error al enviar " << context << ": socket nulo" << std::endl;
        return false;
    }

    if (socket->send(packet) != sf::Socket::Status::Done)
    {
        std::cerr << "Error al enviar " << context << std::endl;
        return false;
    }

    return true;
}

void Server::RemoveClient(std::size_t index)
{
    selector.remove(*clients[index]);
    delete clients[index];
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

void Server::RemoveRoomIfEmpty(const std::string& roomId)
{
    for (auto roomIterator = rooms.begin(); roomIterator != rooms.end(); ++roomIterator)
    {
        if (roomIterator->GetId() == roomId && roomIterator->GetPlayers().empty())
        {
            rooms.erase(roomIterator);
            return;
        }
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
            SendPacket(client, packet, "create_room_error");

            std::cout << "Room id " << roomId << " already exists" << std::endl;
            return;
        }
    }

    Room newRoom;
    newRoom.SetId(roomId);
    newRoom.AddPlayer(client, GetAuthenticatedUsername(client));
    rooms.push_back(newRoom);

    std::cout << "Created room id: " << newRoom.GetId() << std::endl;
    std::cout << "After create, rooms: " << rooms.size() << std::endl;
    std::cout << "Players in room: " << newRoom.GetPlayers().size() << std::endl;

    sf::Packet packet;
    packet << tipoPaquete::CREATE_ROOM_OK << newRoom.GetId();
    SendPacket(client, packet, "create_room_ok");
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

    std::cout << "Trying join room: " << roomId << std::endl;

    for (std::size_t roomIndex = 0; roomIndex < rooms.size(); ++roomIndex)
    {
        Room& room = rooms[roomIndex];

        std::cout << "Checking room id: " << room.GetId()
            << " players: " << room.GetPlayers().size() << std::endl;

        if (room.GetId() != roomId)
        {
            continue;
        }

        if (room.IsFull())
        {
            sf::Packet packet;
            packet << tipoPaquete::JOIN_ROOM_ERROR;
            SendPacket(client, packet, "join_room_error");
            return;
        }

        if (room.HasPlayer(client))
        {
            sf::Packet packet;
            packet << tipoPaquete::JOIN_ROOM_ERROR;
            SendPacket(client, packet, "join_room_error");

            std::cout << "Client already in room " << roomId << std::endl;
            return;
        }

        room.AddPlayer(client, GetAuthenticatedUsername(client));

        std::cout << "Joined room " << roomId
            << ", players now: " << room.GetPlayers().size() << std::endl;

        sf::Packet packet;
        packet << tipoPaquete::JOIN_ROOM_OK << roomId;
        SendPacket(client, packet, "join_room_ok");

        if (room.IsFull())
        {
            std::cout << "Room " << roomId << " is full. Preparing P2P match..." << std::endl;

            SendMatchReady(room);

            // Importante: una vez arranca la partida, esta sala deja de existir
            // en el bootstrap. Asi ya puede volver a crearse otra con el mismo id.
            rooms.erase(rooms.begin() + static_cast<std::ptrdiff_t>(roomIndex));
        }

        return;
    }

    sf::Packet packet;
    packet << tipoPaquete::JOIN_ROOM_ERROR;
    SendPacket(client, packet, "join_room_error");

    std::cout << "Room id " << roomId << " doesn't exist" << std::endl;
}

void Server::SendMatchReady(Room& room)
{
    const std::vector<Player>& players = room.GetPlayers();

    if (players.empty())
    {
        return;
    }

    // Elegimos como host al primer jugador que entro en la sala.
    sf::TcpSocket* hostSocket = players.front().GetSocket();
    const std::string hostUsername = players.front().GetUsername();
    const sf::IpAddress hostAddress = hostSocket->getRemoteAddress();
    const std::uint16_t hostPort = GetPeerPort(hostSocket);

    for (const Player& targetPlayer : players)
    {
        sf::Packet startPacket;
        startPacket << tipoPaquete::START_GAME;
        startPacket << room.GetId();
        startPacket << hostUsername;
        startPacket << hostAddress.toString();
        startPacket << static_cast<std::int32_t>(hostPort);
        startPacket << static_cast<std::int32_t>(players.size());

        // Aprovechamos este paquete para mandar tambien los datos basicos
        // de los jugadores que se veran en la UI del gameplay.
        for (const Player& currentPlayer : players)
        {
            const PlayerData data = databaseManager.GetPlayerbyName(currentPlayer.GetUsername());

            startPacket << currentPlayer.GetUsername();
            startPacket << static_cast<std::int32_t>(data.id);
            startPacket << static_cast<std::int32_t>(data.puntuacion_total);
        }

        SendPacket(targetPlayer.GetSocket(), startPacket, "start_game");
    }

    std::cout << "Datos P2P enviados para la room " << room.GetId()
        << ". Host: " << hostUsername
        << " (" << hostAddress.toString() << ":" << hostPort << ")"
        << std::endl;
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

    SendPacket(&client, packet, "user_info");
}

void Server::ApplyValidatedResult(const std::vector<std::string>& orderedPlayers)
{
    // Mantenemos la misma idea que ya teniais: los ganadores reciben
    // puntos segun su posicion y el ultimo/ultimos pierden puntos.
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
    const auto foundClient = authenticatedClients.find(client);

    if (foundClient == authenticatedClients.end())
    {
        return false;
    }

    return foundClient->second.authenticated;
}

std::string Server::GetAuthenticatedUsername(sf::TcpSocket* client) const
{
    const auto foundClient = authenticatedClients.find(client);

    if (foundClient == authenticatedClients.end())
    {
        return "";
    }

    return foundClient->second.username;
}

std::uint16_t Server::GetPeerPort(sf::TcpSocket* client) const
{
    const auto foundClient = authenticatedClients.find(client);

    if (foundClient == authenticatedClients.end())
    {
        return 0;
    }

    return foundClient->second.peerPort;
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