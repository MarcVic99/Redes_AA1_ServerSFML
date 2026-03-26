#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <cmath>


#define LISTENER_PORT 55000



void SendData(sf::TcpSocket& client, sf::Packet& packet)
{
    if (client.send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "Mensaje enviado" << std::endl;
    }
    else
    {
        std::cerr << "Error al enviar el paquete al cliente" << std::endl;
    }
}

void RandomResponse(sf::TcpSocket& _client, sf::Packet& _packet)
{
    std::srand(static_cast<unsigned>(std::time(0)));
    int random_value = std::rand() % 2;

    if (random_value == 0)
    {
        std::string message;
        message = "HANDSHAKE";

        _packet << tipoPaquete::HANDSHAKE << message;
        SendData(_client, _packet);
    }
    else {
        std::string user = "Marc";
        std::string password = "1234";

        _packet << tipoPaquete::LOGIN << user << password;
        SendData(_client, _packet);
    }
}
//int main()
//{
//    sf::TcpListener listener;
//    sf::TcpSocket client;
//
//    bool closeServer = false;
//
//    while (!closeServer)
//    {
//        if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done)
//        {
//            std::cout << "Error al intentar escuchar en el puerto " << LISTENER_PORT << std::endl;
//            return -1;
//        }
//
//        std::cout << "Esperando conexion..." << std::endl;
//
//        if (listener.accept(client) == sf::Socket::Status::Done)
//        {
//            std::cout << "Cliente conectado desde "
//                << client.getRemoteAddress().value()
//                << std::endl;
//
//            sf::Packet packet;
//
//            //RandomResponse(client, packet);
//        }
//        else
//        {
//            std::cout << "Error al aceptar la conexion" << std::endl;
//            return -1;
//        }
//    }
//    
//    if (closeServer)
//    {
//        closeServer = true;
//
//        client.disconnect();
//        return 0;
//
//    }
//
//}

#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>

#define LISTENER_PORT 55000

int main()
{
   
    for (sf::TcpSocket* client : clients)
    {
        delete client;
    }

    return 0;
