#include "game/GameSession.h"

GameSession::GameSession(const std::string& id, const std::vector<Player>& players)
    : roomId(id),
    players(players),
    currentTurnIndex(0),
    finished(false)
{
    isSpectator.resize(players.size(), false);
    loosers = players;

    AssignColors();
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


bool GameSession::MakeMove(sf::TcpSocket* socket, int row, int col, Cell& cell)
{
    if (finished)
        return false;

    if (!IsPlayerTurn(socket))
        return false;

    if (isSpectator[currentTurnIndex]) {
        std::cout << "Player " << players[currentTurnIndex].GetUsername() << "Already won." << std::endl;
        AdvanceTurn();
    }

    std::cout << "Making move" << std::endl;

    int playerIndex = currentTurnIndex;

    // mapear jugador -> ficha
    cell = static_cast<Cell>(playerIndex + 1);

    if (!board.MakeMove(cell, row, col))
        return false;

    // comprobar victoria
    if (board.CheckWin(row, col, cell))
    {
        winners.push_back(players[playerIndex].GetUsername());
		loosers.erase(loosers.begin() + playerIndex);
        isSpectator[playerIndex] = true;

        if (winners.size() >= 3)
        {
            finished = true;
        }
    }

    AdvanceTurn();

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