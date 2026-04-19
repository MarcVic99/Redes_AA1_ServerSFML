#include "game/GameSession.h"

GameSession::GameSession(const std::string& id, const std::vector<Player>& players)
    : roomId(id),
    players(players),
    currentTurnIndex(0),
    finished(false)
{
    isSpectator.resize(players.size(), false);
    losers = players;

    AssignColors();
    turnClock.restart();
}

bool GameSession::IsPlayerTurn(sf::TcpSocket* socket) const
{
    return players[currentTurnIndex].GetSocket() == socket;
}

bool GameSession::HasPlayer(sf::TcpSocket* socket) const
{
    for (const auto& player : players)
    {
        if (player.GetSocket() == socket)
            return true;
    }
    return false;
}

//random de los colores
void GameSession::AssignColors()
{
    std::vector<PlayerColor> colors =
    {
        PlayerColor::Rojo,
        PlayerColor::Naranja,
        PlayerColor::Verde,
        PlayerColor::Azul
    };

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(colors.begin(), colors.end(), g);

    for (int i = 0; i < players.size(); i++)
    {
        players[i].SetPlayerColor(colors[i]);
    }
}


bool GameSession::SessionMakeMove(sf::TcpSocket* socket, int row, int col, Cell& cell)
{
    if (finished)
        return false;

    if (!IsPlayerTurn(socket))
        return false;

    if (isSpectator[currentTurnIndex])
    {
        std::cout << "Player " << players[currentTurnIndex].GetUsername() << " Already won." << std::endl;
        AdvanceTurn();
        RestartTurnClock();
    }

    std::cout << "Making move" << std::endl;

    int playerIndex = currentTurnIndex;

    cell = static_cast<Cell>(playerIndex + 1);

    if (!board.MakeMove(cell, row, col))
        return false;

    if (board.CheckWin(row, col, cell))
    {
        winners.push_back(players[playerIndex]);

        //borrar de loser el que ha ganado
        for (auto it = losers.begin(); it != losers.end(); ++it)
        {
            if (it->GetUsername() == players[playerIndex].GetUsername())
            {
                losers.erase(it);
                break; 
            }
        }

        isSpectator[playerIndex] = true;

        if (winners.size() >= 3)
        {
            finished = true;
        }
    }

    // empate si el tablero se llena antes de acabar por victorias
    if (!finished && board.CheckDraw())
    {
        draw = true;
        finished = true;
        std::cout << "La partida termina en empate." << std::endl;
    }

    if (!finished)
    {
        AdvanceTurn();
        RestartTurnClock();
    }

    return true;
}

void GameSession::AdvanceTurn()
{
    int count = players.size();

    for (int i = 0; i < count; i++)
    {
        currentTurnIndex = (currentTurnIndex + 1) % count;

        if (!isSpectator[currentTurnIndex])
            return;
    }
}



//Temporizador funciones
void GameSession::RestartTurnClock()
{
    turnClock.restart();
}

bool GameSession::HasTurnTimedOut() const
{
    return turnClock.getElapsedTime().asSeconds() >= TURN_LIMIT_SECONDS;
}

bool GameSession::UpdateTurnTimeout()
{
    if (finished)
        return false;

    if (!HasTurnTimedOut())
        return false;

    std::cout << "Turno agotado para jugador: "
        << players[currentTurnIndex].GetUsername() << std::endl;

    AdvanceTurn();
    RestartTurnClock();

    return true;
}