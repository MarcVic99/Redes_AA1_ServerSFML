#include "network/Server.h"

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

        for (GameSession& session : sessions)
        {
            if (session.UpdateTurnTimeout())
            {
                std::cout << "Turno saltado por timeout en room: " << session.GetRoomId() << std::endl;
                BroadcastSkipTurnTimeout(&session);
            }
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
    std::cout << "Servidor escuchando en puerto " << kListenerPort << std::endl;
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

            case tipoPaquete::CREATE_ROOM:
            {
                std::string username;
                std::string roomId;
                packet >> roomId >> username;
                CreateRoom(client, username, roomId);
                break;
            }

            case tipoPaquete::JOIN_ROOM:
            {
                std::string roomId;
                std::string username;
                packet >> roomId >> username;
                JoinRoom(client, roomId, username);
                break;
            }

            case tipoPaquete::PLAYERS_GAME_REQUEST:
            {
                GameSession* session = GetSessionByClient(client);
                std::cout << "Client requests players in game. Session found: " << (session != nullptr) << std::endl;

                if (session == nullptr)
                {
                    std::cout << "Session not found for client" << std::endl;
                    break;
                }

                SendPlayers(client, session->GetPlayers());
                break;
            }

            case tipoPaquete::PLAYER_MOVE:
            {
                GameSession* session = GetSessionByClient(client);

                if (session == nullptr)
                {
                    std::cout << "Session not found for client" << std::endl;
                    break;
                }

                std::int32_t row = 0;
                std::int32_t column = 0;
                std::string username;

                packet >> username >> row >> column;

                Cell cell = Cell::Empty;

                if (!session->SessionMakeMove(client, row, column, cell))
                {
                    std::cout << "Invalid move from client " << username << ": row=" << row << " column=" << column << std::endl;

                    if (session->IsFinished())
                    {
                        const std::vector<Player>& winners = session->GetWinners();
                        std::cout << "Game finished. Winners: ";
                        for (const Player& winner : winners)
                        {
                            std::cout << winner.GetUsername() << ' ';
                        }
                        std::cout << std::endl;
                    }
                    break;
                }

                BroadcastPlayerMove(session, client, cell, row, column);

                if (session->IsFinished())
                {
                    CheckFinish(session->GetPlayers(), session->GetWinners(), session->GetLosers(), session->IsFinished());
                    std::cout << "Partida finalizada del todo" << std::endl;
                    break;
                }

                std::cout << "Player move: row=" << row << " column=" << column << " cell=" << static_cast<int>(cell) << std::endl;
                break;
            }

            default:
                break;
            }
        }
        else if (status == sf::Socket::Status::Disconnected)
        {
            std::cout << "Cliente desconectado" << std::endl;

            for (Room& room : rooms)
            {
                room.RemovePlayer(client);
            }

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
    std::cout << "Login recibido:" << std::endl
              << "User: " << username << " Password: " << password << std::endl;

    sf::Packet response;
    if (databaseManager.ValidateLogin(username, password))
    {
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

    SendPlayerInfo(client, username);
}

void Server::HandleRegister(sf::TcpSocket& client, sf::Packet& packet)
{
    static const std::string kClientCreatedMessage = "CLIENT_CREATED";
    static const std::string kRegisterErrorMessage = "REGISTER_ERROR";

    std::string username;
    std::string password;

    packet >> username >> password;
    std::cout << "Registro recibido:" << std::endl
              << "User: " << username << " Password: " << password << std::endl;

    sf::Packet response;
    if (databaseManager.RegisterUser(username, password))
    {
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

    SendPlayerInfo(client, username);
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

    for (const PlayerData& playerData : topPlayers)
    {
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

void Server::CreateRoom(sf::TcpSocket* client, const std::string& username, const std::string& roomId)
{
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
    newRoom.AddPlayer(client, username);
    rooms.push_back(newRoom);

    std::cout << "Created room id: " << newRoom.GetId() << std::endl;
    std::cout << "After create, rooms: " << rooms.size() << std::endl;
    std::cout << "Players in room: " << newRoom.GetPlayers().size() << std::endl;

    sf::Packet packet;
    packet << tipoPaquete::CREATE_ROOM_OK << newRoom.GetId();
    SendPacket(client, packet, "create_room_ok");
}

void Server::JoinRoom(sf::TcpSocket* client, const std::string& roomId, const std::string& username)
{
    std::cout << "Trying join room: " << roomId << std::endl;

    for (Room& room : rooms)
    {
        std::cout << "Checking room id: " << room.GetId() << " players: " << room.GetPlayers().size() << std::endl;

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

        room.AddPlayer(client, username);

        std::cout << "Joined room " << roomId << ", players now: " << room.GetPlayers().size() << std::endl;

        sf::Packet packet;
        packet << tipoPaquete::JOIN_ROOM_OK << roomId;
        SendPacket(client, packet, "join_room_ok");

        if (room.IsFull())
        {
            std::cout << "Room " << roomId << " is full. Starting game..." << std::endl;

            sessions.emplace_back(room.GetId(), room.GetPlayers());

            sf::Packet startPacket;
            startPacket << tipoPaquete::START_GAME
                        << room.GetId()
                        << static_cast<std::int32_t>(room.GetPlayers().size());

            for (const Player& player : room.GetPlayers())
            {
                SendPacket(player.GetSocket(), startPacket, "start_game");
            }
        }

        return;
    }

    sf::Packet packet;
    packet << tipoPaquete::JOIN_ROOM_ERROR;
    SendPacket(client, packet, "join_room_error");
    std::cout << "Room id " << roomId << " doesn't exist" << std::endl;
}

void Server::SendPlayers(sf::TcpSocket* client, const std::vector<Player>& players)
{
    sf::Packet packet;

    packet << tipoPaquete::PLAYERS_GAME_RESPONSE;
    packet << static_cast<std::int32_t>(players.size());

    for (const auto& player : players)
    {
        PlayerData data = databaseManager.GetPlayerbyName(player.GetUsername());

        packet << player.GetUsername();
        packet << static_cast<std::int32_t>(data.puntuacion_total);
        packet << static_cast<std::int32_t>(player.GetPlayerColor());

        std::cout << "SendPlayers -> "
            << player.GetUsername()
            << " score=" << data.puntuacion_total
            << " color=" << static_cast<int>(player.GetPlayerColor())
            << std::endl;
    }

    SendPacket(client, packet, "players_game_response");
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
    packet << tipoPaquete::USER_INFO << static_cast<std::int32_t>(playerData.id) << playerData.user;

    std::cout << "Sending player info to client: ID=" << playerData.id
              << " Username='" << playerData.user
              << "' size=" << playerData.user.size()
              << std::endl;

    SendPacket(&client, packet, "user_info");
}

GameSession* Server::GetSessionByClient(sf::TcpSocket* client)
{
    for (GameSession& session : sessions)
    {
        if (session.HasPlayer(client))
        {
            return &session;
        }
    }

    return nullptr;
}

void Server::BroadcastPlayerMove(GameSession* session, sf::TcpSocket* sender, Cell cell, int row, int column)
{
    sf::Packet packet;
    packet << tipoPaquete::BROADCAST_PLAYER_MOVE
           << static_cast<std::int32_t>(cell)
           << row
           << column
           << static_cast<std::int32_t>(session->GetCurrentTurnIndex());

    for (const Player& player : session->GetPlayers())
    {
        sf::TcpSocket* target = player.GetSocket();

        if (target == sender)
        {
            continue;
        }

        SendPacket(target, packet, "broadcast_player_move");
        std::cout << "Broadcasting move to player " << player.GetUsername()
                  << ": cell=" << static_cast<int>(cell)
                  << " row=" << row
                  << " column=" << column
                  << std::endl;
    }
}

void Server::BroadcastSkipTurnTimeout(GameSession* session)
{
    sf::Packet packet;
    packet << tipoPaquete::SKIP_TURN << static_cast<std::int32_t>(session->GetCurrentTurnIndex());

    for (const Player& player : session->GetPlayers())
    {
        SendPacket(player.GetSocket(), packet, "turn_timeout");
    }
}

void Server::CheckFinish(const std::vector<Player>& players, const std::vector<Player>& winners, const std::vector<Player>& losers, bool isFinished)
{
    if (!isFinished)
    {
        std::cout << "No ha terminado todavia" << std::endl;
        return;
    }

    sf::Packet finishedPacket;
    finishedPacket << tipoPaquete::GAME_FINISHED;

    const std::int32_t totalPlayers = static_cast<std::int32_t>(winners.size() + losers.size());
    finishedPacket << totalPlayers;

    std::size_t winnerIndex = 0;
    for (const Player& winner : winners)
    {
        databaseManager.UpdatePlayerStats(winner.GetUsername(), kWinnerPoints[winnerIndex], true, false);
        finishedPacket << winner.GetUsername();
        finishedPacket << static_cast<std::int32_t>(kWinnerPoints[winnerIndex]);
        ++winnerIndex;
    }

    for (const Player& loser : losers)
    {
        databaseManager.UpdatePlayerStats(loser.GetUsername(), kLoserPoints, false, true);
        finishedPacket << loser.GetUsername();
        finishedPacket << static_cast<std::int32_t>(kLoserPoints);
    }

    for (const Player& player : players)
    {
        SendPacket(player.GetSocket(), finishedPacket, "game_finished");
    }
}

void Server::Shutdown()
{
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }

    clients.clear();
}
