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
                std::string message;
                packet >> message;
                std::cout << "Mensaje: " << message << std::endl;
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

void Server::removeClient(std::size_t index)
{
    selector.remove(*clients[index]);
    delete clients[index];
    clients.erase(clients.begin() + index);
}

void Server::shutdown()
{
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }
    clients.clear();
}

