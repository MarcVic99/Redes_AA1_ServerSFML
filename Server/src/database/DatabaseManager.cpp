#include "database/DatabaseManager.h"

#include "core/Constants.h"

#include <array>

DatabaseManager::DatabaseManager()
    : driver(nullptr), connection(nullptr)
{
}

DatabaseManager::~DatabaseManager()
{
    Disconnect();
}

bool DatabaseManager::Connect()
{
    try
    {
        driver = get_driver_instance();
        connection = driver->connect(DATABASE_SERVER, DATABASE_USERNAME, DATABASE_PASSWORD);
        connection->setSchema(DATABASE_NAME);

        std::cout << "Connection done." << std::endl;
        return true;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Could not connect to server. Error message: " << exception.what() << std::endl;
        return false;
    }
}

void DatabaseManager::Disconnect()
{
    if (connection == nullptr)
    {
        return;
    }

    connection->close();

    if (connection->isClosed())
    {
        std::cout << "Connection closed" << std::endl;
        delete connection;
        connection = nullptr;
        driver = nullptr;
    }
}

bool DatabaseManager::RegisterUser(const std::string& username, const std::string& password)
{
    if (connection == nullptr || username.empty() || password.empty())
    {
        return false;
    }

    try
    {
        if (CheckUserExists(username))
        {
            return false;
        }

        sql::PreparedStatement* statement = connection->prepareStatement(
            "INSERT INTO users (user, password) VALUES (?, ?)"
        );

        statement->setString(1, username);
        statement->setString(2, HashPassword(password));

        const int affectedRows = statement->executeUpdate();
        std::cout << "Number of rows affected: " << affectedRows << std::endl;

        delete statement;
        return affectedRows > 0;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error while creating user: " << exception.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::CheckUserExists(const std::string& username)
{
    if (connection == nullptr || username.empty())
    {
        return false;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "SELECT user FROM users WHERE user = ?"
        );

        statement->setString(1, username);
        sql::ResultSet* result = statement->executeQuery();

        const bool exists = result->next();

        delete result;
        delete statement;

        return exists;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error while checking the user: " << exception.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::ValidateLogin(const std::string& username, const std::string& password)
{
    if (connection == nullptr || username.empty() || password.empty())
    {
        return false;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "SELECT password FROM users WHERE user = ?"
        );

        statement->setString(1, username);

        sql::ResultSet* result = statement->executeQuery();

        if (!result->next())
        {
            delete result;
            delete statement;
            return false;
        }

        const std::string storedHash = result->getString("password");

        delete result;
        delete statement;

        return VerifyPassword(password, storedHash);
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error while validating login: " << exception.what() << std::endl;
        return false;
    }
}

std::vector<PlayerData> DatabaseManager::GetTopPlayers()
{
    std::vector<PlayerData> players;

    if (connection == nullptr)
    {
        return players;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "SELECT user, puntuacion_total, victorias, derrotas "
            "FROM users "
            "ORDER BY puntuacion_total DESC "
            "LIMIT ?"
        );
        statement->setInt(1, kTopRankingLimit);

        sql::ResultSet* result = statement->executeQuery();

        while (result->next())
        {
            PlayerData playerData;
            playerData.user = result->getString("user");
            playerData.puntuacion_total = result->getInt("puntuacion_total");
            playerData.victorias = result->getInt("victorias");
            playerData.derrotas = result->getInt("derrotas");

            players.push_back(playerData);
        }

        delete result;
        delete statement;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error getting top users: " << exception.what() << std::endl;
    }

    return players;
}

PlayerData DatabaseManager::GetPlayerById(int id)
{
    PlayerData playerData;

    if (connection == nullptr)
    {
        return playerData;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "SELECT id, user, puntuacion_total, victorias, derrotas "
            "FROM users WHERE id = ?"
        );

        statement->setInt(1, id);
        sql::ResultSet* result = statement->executeQuery();

        if (result->next())
        {
            playerData.id = result->getInt("id");
            playerData.user = result->getString("user");
            playerData.puntuacion_total = result->getInt("puntuacion_total");
            playerData.victorias = result->getInt("victorias");
            playerData.derrotas = result->getInt("derrotas");
        }

        delete result;
        delete statement;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error getting user by id: " << exception.what() << std::endl;
    }

    return playerData;
}

PlayerData DatabaseManager::GetPlayerbyName(const std::string& name)
{
    PlayerData data;

    if (connection == nullptr)
        return data;

    try
    {
        sql::PreparedStatement* stmt = connection->prepareStatement(
            "SELECT id, user, puntuacion_total, victorias, derrotas FROM users WHERE user = ?"
        );

        stmt->setString(1, name);
        sql::ResultSet* res = stmt->executeQuery();

        if (res->next())
        {
            data.id = res->getInt("id");
            data.user = res->getString("user");
            data.puntuacion_total = res->getInt("puntuacion_total");
            data.victorias = res->getInt("victorias");
            data.derrotas = res->getInt("derrotas");
        }

        delete res;
        delete stmt;
    }
    catch (sql::SQLException e)
    {
        std::cout << "Error GetPlayerByName: " << e.what() << std::endl;
    }

    return data;
}

int DatabaseManager::GetPlayerRank(int id)
{
    int rank = -1;

    if (connection == nullptr)
    {
        return rank;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "SELECT COUNT(*) + 1 AS posicion "
            "FROM users "
            "WHERE puntuacion_total > (SELECT puntuacion_total FROM users WHERE id = ?)"
        );
        statement->setInt(1, id);

        sql::ResultSet* result = statement->executeQuery();

        if (result->next())
        {
            rank = result->getInt("posicion");
        }

        delete result;
        delete statement;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error getting user rank: " << exception.what() << std::endl;
    }

    return rank;
}

bool DatabaseManager::UpdatePlayerStats(const std::string& username, int pointsToAdd, bool addVictory, bool addDefeat)
{
    if (connection == nullptr || username.empty())
    {
        return false;
    }

    try
    {
        sql::PreparedStatement* statement = connection->prepareStatement(
            "UPDATE users "
            "SET puntuacion_total = puntuacion_total + ?, "
            "victorias = victorias + ?, "
            "derrotas = derrotas + ? "
            "WHERE user = ?"
        );

        statement->setInt(1, pointsToAdd);
        statement->setInt(2, addVictory ? 1 : 0);
        statement->setInt(3, addDefeat ? 1 : 0);
        statement->setString(4, username);

        const int affectedRows = statement->executeUpdate();

        delete statement;

        std::cout << "Stats actualizadas para: " << username
                  << " | puntos " << pointsToAdd
                  << " | victoria: " << addVictory
                  << " | derrota: " << addDefeat
                  << std::endl;

        return affectedRows > 0;
    }
    catch (const sql::SQLException& exception)
    {
        std::cout << "Error updating player stats: " << exception.what() << std::endl;
        return false;
    }
}

std::string DatabaseManager::HashPassword(const std::string& password)
{
    std::array<char, crypto_pwhash_STRBYTES> hashedPassword{};

    if (crypto_pwhash_str(
            hashedPassword.data(),
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        return "";
    }

    return std::string(hashedPassword.data());
}

bool DatabaseManager::VerifyPassword(const std::string& password, const std::string& storedHash)
{
    return crypto_pwhash_str_verify(
               storedHash.c_str(),
               password.c_str(),
               password.size()) == 0;
}
