#include "database/DatabaseManager.h"

#include "core/Constants.h"

#include <sodium.h>

#include <array>
#include <string>

namespace
{
    constexpr size_t kPasswordHashBytes = 32;
    constexpr char kPasswordHashSeparator = ':';

    std::string ToHex(const unsigned char* data, size_t size)
    {
        std::string hex(size * 2 + 1, '\0');
        sodium_bin2hex(hex.data(), hex.size(), data, size);
        hex.pop_back();
        return hex;
    }

    bool FromHex(const std::string& hex, unsigned char* output, size_t outputSize)
    {
        size_t binaryLength = 0;

        return sodium_hex2bin(
            output,
            outputSize,
            hex.c_str(),
            hex.size(),
            nullptr,
            &binaryLength,
            nullptr) == 0 && binaryLength == outputSize;
    }

    bool VerifyLibsodiumFormattedHash(const std::string& password, const std::string& storedHash)
    {
        std::string fullHash = storedHash;

        if (fullHash.rfind("$argon2", 0) != 0)
        {
            fullHash = std::string(crypto_pwhash_STRPREFIX) + fullHash;
        }

        return crypto_pwhash_str_verify(
            fullHash.c_str(),
            password.c_str(),
            password.size()) == 0;
    }
}


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
    if (sodium_init() < 0)
    {
        std::cout << "Could not initialize libsodium." << std::endl;
        return false;
    }

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

        const std::string passwordHash = HashPassword(password);

        if (passwordHash.empty())
        {
            std::cout << "Error while hashing password." << std::endl;
            return false;
        }

        sql::PreparedStatement* statement = connection->prepareStatement(
            "INSERT INTO users (user, password) VALUES (?, ?)"
        );

        statement->setString(1, username);
        statement->setString(2, passwordHash);

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
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
    std::array<unsigned char, kPasswordHashBytes> hash{};

    randombytes_buf(salt.data(), salt.size());

    if (crypto_pwhash(
            hash.data(),
            hash.size(),
            password.c_str(),
            password.size(),
            salt.data(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE,
            crypto_pwhash_ALG_ARGON2ID13) != 0)
    {
        return "";
    }

    return ToHex(salt.data(), salt.size()) + kPasswordHashSeparator + ToHex(hash.data(), hash.size());
}

bool DatabaseManager::VerifyPassword(const std::string& password, const std::string& storedHash)
{
    if (password.empty() || storedHash.empty())
    {
        return false;
    }

    // Compatibilidad con hashes antiguos guardados en formato libsodium:
    // "$argon2id$v=19$m=..." o "v=19$m=..." si previamente quitaste solo el prefijo.
    if (storedHash.rfind("$argon2", 0) == 0 || storedHash.rfind("v=", 0) == 0)
    {
        return VerifyLibsodiumFormattedHash(password, storedHash);
    }

    const size_t separator = storedHash.find(kPasswordHashSeparator);

    if (separator == std::string::npos)
    {
        return false;
    }

    const std::string saltHex = storedHash.substr(0, separator);
    const std::string hashHex = storedHash.substr(separator + 1);

    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
    std::array<unsigned char, kPasswordHashBytes> storedHashBytes{};
    std::array<unsigned char, kPasswordHashBytes> newHash{};

    if (!FromHex(saltHex, salt.data(), salt.size()))
    {
        return false;
    }

    if (!FromHex(hashHex, storedHashBytes.data(), storedHashBytes.size()))
    {
        return false;
    }

    if (crypto_pwhash(
            newHash.data(),
            newHash.size(),
            password.c_str(),
            password.size(),
            salt.data(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE,
            crypto_pwhash_ALG_ARGON2ID13) != 0)
    {
        return false;
    }

    return sodium_memcmp(
               newHash.data(),
               storedHashBytes.data(),
               kPasswordHashBytes) == 0;
}
