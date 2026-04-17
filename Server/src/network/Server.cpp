#include "network/Server.h"
#include <iostream>
#include <string>

int Server::run()
{
    bool serverClosed = false;

    if (!InitializeListener())
    {
        std::cout << "Error listener" << std::endl;
        return -1;
    }

    if (!databaseManager.ConnectDB())
    {
        std::cout << "Error conectando con DB" << std::endl;
        return -1;
    }

    // Libsodium para el hash de password  
    if (sodium_init() < 0)
    {
        std::cout << "Error inicializando libsodium" << std::endl;
        return -1;
    }

    while (!serverClosed)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                HandleNewConnection();
            }
            else
            {
                HandleClientMessages();
            }
        }
    }
    databaseManager.disconnectDB();
    Shutdown();
    return 0;
}



//El server empieza a escuchar 
bool Server::InitializeListener()
{
    if (listener.listen(LISTENER_PORT, sf::IpAddress::Any) != sf::Socket::Status::Done) 
        {
        std::cout << "Error al intentar escuchar en el puerto " << LISTENER_PORT << std::endl;
        return false;
    }

    selector.add(listener);
    std::cout << "Servidor escuchando en puerto " << LISTENER_PORT << std::endl;
    return true;
}



//Crea un nuevo cliente 
  // y si es aceptado se añade al vector y al selector. 
  // Sino, se borra
void Server::HandleNewConnection() 
{
  
    sf::TcpSocket* newClient = new sf::TcpSocket();

    if (listener.accept(*newClient) == sf::Socket::Status::Done)
    {
        newClient->setBlocking(false);
        selector.add(*newClient);
        clients.push_back(newClient);
        std::cout << "Nueva conexion establecida" << std::endl;
    }
    else
    {
        delete newClient;
    }
}



//Por cada cliente, se comprueba que tipo de paquete ha llegado y gestiona
void Server::HandleClientMessages()
{
    for (int i = 0; i < static_cast<int>(clients.size()); i++)
    {
        if (selector.isReady(*clients[i]))
        {
            sf::Packet packet;
            sf::Socket::Status status = clients[i]->receive(packet);

            if (status == sf::Socket::Status::Done)
            {
                tipoPaquete tipo;
                packet >> tipo;

                switch (tipo)
                {
                case tipoPaquete::HANDSHAKE:
                    HandleHandshake(*clients[i], packet);
                    break;

                case tipoPaquete::LOGIN:
                    HandleLogin(*clients[i], packet);
                    break;

                case tipoPaquete::REGISTER:
                    HandleRegister(*clients[i], packet);
                    break;
                case tipoPaquete::GET_RANKING:
                    HandleGetRanking(*clients[i], packet);
                    break;
                case tipoPaquete::CREATE_ROOM:
                {
                    std::string username;
                    std::string roomId;
                    packet >> roomId >> username;
                    CreateRoom(clients[i], username, roomId);
                    break;
                }

                case tipoPaquete::JOIN_ROOM:
                {
                    std::string roomId;
                    std::string username;

                    packet >> roomId >> username;

                    JoinRoom(clients[i], roomId, username);
                    break;
                }
                case tipoPaquete::PLAYERS_GAME_REQUEST:
                {
                    GameSession* session = GetSessionByClient(clients[i]);

					std::cout << "Client requests players in game. Session found: " << (session != nullptr) << std::endl;

                    if (session == nullptr)
                    {
                        std::cout << "Session not found for client" << std::endl;
                        break;
                    }

                    SendPlayers(clients[i], session->GetPlayers());
                    break;
                }

                default:
                    break;
                }
            }

            else if (status == sf::Socket::Status::Disconnected)
            {
                std::cout << "Cliente desconectado" << std::endl;

                // quitar de rooms
                for (auto& room : _rooms)
                {
                    room.RemovePlayer(clients[i]);
                }

                // (futuro) quitar de partidas activas
                // for (auto& session : sessions)
                // {
                //     session.RemovePlayer(clients[i]);
                // }

                std::cout << "Cliente desconectado" << std::endl;
                RemoveClient(i);
                i--;
            }
        }
    }
}




//El server recibe un mensaje de Handshake y comprueba si el mensaje es el correcto.
// Si es así, envia mensaje de Hello_Client y si es otro, Hello_Error
void Server::HandleHandshake(sf::TcpSocket& client, sf::Packet& packet)
{
    //Leemos paquete de handshake
    std::string message;
    packet >> message;
    std::cout << "Handshake recibido: " << message << std::endl;

    //Creamos paquete
    sf::Packet response;
    if (message == "HELLO_SERVER")
    {
        response << tipoPaquete::HANDSHAKE_OK << std::string("HELLO_CLIENT");
    }
    else {
        response << tipoPaquete::HANDSHAKE_ERROR << std::string("HELLO_ERROR");
    }

    //Enviamos paquete de handshake o error
    if (!SendPacket(client, response, "handsahake"))
    {
        return;
    }
}

void Server::HandleLogin(sf::TcpSocket& client, sf::Packet packet)
{
    std::string user;
    std::string password;

    packet >> user;
    packet >> password;
    std::cout << "Login recibido:" << std::endl << "User: " << user << " Password: " << password << std::endl;

    //Creamos paquete
    sf::Packet response;

    if (databaseManager.ValidateLogin(user, password))
    {    
        response << tipoPaquete::LOGIN_OK << std::string("CLIENT_VERIFIED");
    }else {
        response << tipoPaquete::LOGIN_ERROR << std::string("LOGIN_ERROR");
    }
    //Enviamos paquete de login o error
    if (!SendPacket(client, response, "login"))
    {
        return;
    }

    SendPlayerInfo(&client, user);
   
}

void Server::HandleRegister(sf::TcpSocket& client, sf::Packet packet)
{
    std::string user;
    std::string password;

    packet >> user;
    packet >> password;
    std::cout << "Registro recibido:" << std::endl << "User: " << user << " Password: " << password;

    //Creamos paquete
    sf::Packet response;

    if (databaseManager.RegisterUserDB(user, password))
    {
        response << tipoPaquete::REGISTER_OK << std::string("CLIENT_CREATED");
    }
    else {
        response << tipoPaquete::REGISTER_ERROR << std::string("REGISTER_ERROR");
    }

    //Enviamos paquete de register o error
    if (!SendPacket(client, response, "register"))
    {
        return;
    }

    SendPlayerInfo(&client, user);
}


void Server::HandleGetRanking(sf::TcpSocket& client, sf::Packet packet)
{
    //el cliente debería enviar su id para el 11º puesto
    int userID;

    packet >> userID;

    std::cout << "Cliente pide acceso al Ranking. Cliente ID:" << userID << std::endl;

    //ordenados del 1 al 10
    std::vector<PlayerData> top10Users = databaseManager.GetTop10();

    sf::Packet response;
    response << tipoPaquete::RECEIVE_RANKING;

    //tamaño del vector
    response << static_cast<int>(top10Users.size());

    //cada user
    for (const auto& cliente : top10Users)
    {
        response << cliente.user;
        response << cliente.puntuacion_total;
    }

    //añadir el ranking del cliente actual
    int userRank = databaseManager.GetPlayerRank(userID);
    response << userRank;

    if (!SendPacket(client, response, "getRegister"))
    {
        return;
    }

}


bool Server::SendPacket(sf::TcpSocket& socket, sf::Packet& packet, const std::string& context)
{
    if (socket.send(packet) != sf::Socket::Status::Done)
    {
        std::cerr << "Error al enviar " << context << std::endl;
        return false;
    }
    return true;
}


void Server::RemoveClient(std::size_t index)
{
    //Se elimina el cliente del selector y del vector cuando se desconecta
    selector.remove(*clients[index]);
    delete clients[index];
    clients.erase(clients.begin() + index);
}


void Server::CreateRoom(sf::TcpSocket* client, const std::string& username, std::string roomId)
{
    Room room;
    room.SetId(roomId);

    for (auto& room : _rooms) {
        if (room.GetId() == roomId) {
            sf::Packet packet;
            packet << tipoPaquete::CREATE_ROOM_ERROR;
            client->send(packet);
            std::cout << "Room id " << roomId << " already exists" << std::endl;
            return;
		}
    }

    room.AddPlayer(client, username);

    _rooms.push_back(room);

    std::cout << "Created room id: " << room.GetId() << std::endl;
    std::cout << "After create, rooms: " << _rooms.size() << std::endl;
    std::cout << "Players in room: " << room.GetPlayers().size() << std::endl;

    sf::Packet packet;
    packet << tipoPaquete::CREATE_ROOM_OK << room.GetId();

    client->send(packet);
}


void Server::JoinRoom(sf::TcpSocket* client, std::string roomId, std::string& username)
{
    std::cout << "Trying join room: " << roomId << std::endl;
    for (int i = 0; i < _rooms.size(); i++)
    {
        Room& room = _rooms[i];

        std::cout << "Checking room id: " << room.GetId()
            << " players: " << room.GetPlayers().size() << std::endl;

        if (room.GetId() == roomId)
        {
            if (room.GetPlayers().size() >= room.GetMaxPlayers())
            {
                sf::Packet packet;
                packet << tipoPaquete::JOIN_ROOM_ERROR;
                client->send(packet);
                return;
            }

            for (auto& player : room.GetPlayers())
            {
                if (player.GetSocket() == client)
                {
                    sf::Packet packet;
                    packet << tipoPaquete::JOIN_ROOM_ERROR;
                    client->send(packet);
                    std::cout << "Client already in room " << roomId << std::endl;
                    return;
                }
            }

            room.AddPlayer(client, username);

            std::cout << "Joined room " << roomId
                << ", players now: " << room.GetPlayers().size() << std::endl;

            sf::Packet packet;
            packet << tipoPaquete::JOIN_ROOM_OK << roomId;
            client->send(packet);

            if (room.IsFull())
            {
                std::cout << "Room " << roomId << " is full. Starting game..." << std::endl;

                GameSession session(room.GetId(), room.GetPlayers());
                _sessions.push_back(session);

                sf::Packet startPacket;
                startPacket << tipoPaquete::START_GAME
                    << room.GetId()
                    << static_cast<int>(room.GetPlayers().size());

                for (const auto& player : room.GetPlayers())
                {
                    player.GetSocket()->send(startPacket);
                }

                //Borramos room
                _rooms.erase(_rooms.begin() + i);
            }

            return;
        }
    }
}

void Server::SendPlayers(sf::TcpSocket* client, const std::vector <Player>& players)
{
    sf::Packet packet;

    packet << tipoPaquete::PLAYERS_GAME_RESPONSE;

    // número de jugadores
    packet << static_cast<std::int32_t>(players.size());

    // enviar username + color
    for (const auto& player : players)
    {
        packet << player.GetUsername();
        
        packet << static_cast<std::int32_t>(player.GetPlayerColor());
    }

    client->send(packet);
}

void Server::SendPlayerInfo(sf::TcpSocket* client, std::string username)
{
    PlayerData data = databaseManager.GetPlayerbyName(username);

    if (data.user.empty())
    {
        std::cout << "Error: user not found in DB\n";
        return;
    }

    sf::Packet packet;
    packet << tipoPaquete::USER_INFO << static_cast<std::int32_t>(data.id) << data.user;

    std::cout << "Sending player info to client: ID=" << data.id << " Username='" << data.user << "' size=" << data.user.size() << std::endl;

	client->send(packet);

    /*if(!SendPacket(client, packet, "user_info"))
        return;*/
}

GameSession* Server::GetSessionByClient(sf::TcpSocket* client)
{
    for (auto& session : _sessions)
    {
        if (session.HasPlayer(client))
            return &session;
    }
    return nullptr;
}


void Server::Shutdown()
{
    //Server cerrado
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }
    clients.clear();
}