#include "network/Server.h"
//#include "network/PacketTypes.h"
#include <iostream>
#include <string>

int Server::run()
{
    bool serverClosed = false;
    
        if (!initializeListener())
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

    if (client.send(response) != sf::Socket::Status::Done)
    {
        std::cout << "Error al enviar mensaje en Handshake" << std::endl;
    }
}

void Server::handleLogin(sf::TcpSocket& client, sf::Packet packet)
{
    std::string user;
    std::string password;

    packet >> user;
    packet >> password;
    std::cout << "Login recibido:" << std::endl << "User: " << user << " Password: " << password;

    
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

