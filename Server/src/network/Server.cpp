#include "network/Server.h"
#include <iostream>
#include <string>

int Server::run()
{
    bool serverClosed = false;
    
    if (!initializeListener())
    {
        return -1;
    }

    if (!databaseManager.ConnectDB())
    {
        return -1;
    }

    while (!serverClosed)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                handleNewConnection();
            }
            else
            {
                handleClientMessages();
            }
        }
    }
    databaseManager.disconnectDB();
    shutdown();
    return 0;
}



//El server empieza a escuchar 
bool Server::initializeListener()
{
    if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done)
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
void Server::handleNewConnection() 
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
void Server::handleClientMessages()
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
                        handleHandshake(*clients[i], packet);
                        break;

                    case tipoPaquete::LOGIN:
                        handleLogin(*clients[i], packet);
                        break;

                    case tipoPaquete::REGISTER:
                        handleRegister(*clients[i], packet);
                        break;
                    case tipoPaquete::GET_RANKING:
                        handleGetRanking(*clients[i], packet);
                        break;

                    default:
                        break;
                }
            }
            else if (status == sf::Socket::Status::Disconnected)
            {
                std::cout << "Cliente desconectado" << std::endl;
                removeClient(i);
                i--;
            }
        }
    }
}




//El server recibe un mensaje de Handshake y comprueba si el mensaje es el correcto.
// Si es así, envia mensaje de Hello_Client y si es otro, Hello_Error
void Server::handleHandshake(sf::TcpSocket& client, sf::Packet& packet)
{
    std::string message;
    packet >> message;
    std::cout << "Handshake recibido: " << message << std::endl;

    sf::Packet response;
    if (message == "HELLO_SERVER")
    {
        response << tipoPaquete::HANDSHAKE_OK << std::string("HELLO_CLIENT");
    }
    else {
        response << tipoPaquete::HANDSHAKE_ERROR << std::string("HELLO_ERROR");
    }

    if (!sendPacket(client, response, "handsahake"))
    {
        return;
    }
}

void Server::handleLogin(sf::TcpSocket& client, sf::Packet packet)
{
    std::string user;
    std::string password;

    packet >> user;
    packet >> password;
    std::cout << "Login recibido:" << std::endl << "User: " << user << " Password: " << password;

    sf::Packet response;

    if (databaseManager.ValidateLogin(user, password))
    {    
        response << tipoPaquete::LOGIN_OK << std::string("CLIENT_VERIFIED");
    }else {
        response << tipoPaquete::LOGIN_ERROR << std::string("LOGIN_ERROR");
    }

    if (!sendPacket(client, response, "login"))
    {
        return;
    }
   
}

void Server::handleRegister(sf::TcpSocket& client, sf::Packet packet)
{
    std::string user;
    std::string password;

    packet >> user;
    packet >> password;
    std::cout << "Registro recibido:" << std::endl << "User: " << user << " Password: " << password;

    databaseManager.RegisterUserDB(user, password);

    sf::Packet response;
    response << tipoPaquete::REGISTER_OK << std::string("CLIENT_CREATED");

    if (!sendPacket(client, response, "register"))
    {
        return;
    }
}


void Server::handleGetRanking(sf::TcpSocket& client, sf::Packet packet)
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

    if (!sendPacket(client, response, "getRegister"))
    {
        return;
    }

}


bool Server::sendPacket(sf::TcpSocket& socket, sf::Packet& packet, const std::string& context)
{
    if (socket.send(packet) != sf::Socket::Status::Done)
    {
        std::cerr << "Error al enviar " << context << std::endl;
        return false;
    }
    return true;
}


void Server::removeClient(std::size_t index)
{
    //Se elimina el cliente del selector y del vector cuando se desconecta
    selector.remove(*clients[index]);
    delete clients[index];
    clients.erase(clients.begin() + index);
}

void Server::shutdown()
{
    //Server cerrado
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }
    clients.clear();
}

