#pragma once

#include "DatabaseConfig.h"

#include <iostream>
#include <string>
#include <vector>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <sodium.h>

struct PlayerData
{
    int id = -1;
    std::string user;
    int puntuacion_total = 0;
    int victorias = 0;
    int derrotas = 0;
};

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    /** Opens the database connection used by the server. */
    bool Connect();

    /** Closes the database connection if it is open. */
    void Disconnect();

    /** Registers a new user in the database. */
    bool RegisterUser(const std::string& username, const std::string& password);

    /** Returns true when a username already exists in the database. */
    bool CheckUserExists(const std::string& username);

    /** Validates the provided credentials against the stored hash. */
    bool ValidateLogin(const std::string& username, const std::string& password);

    /** Returns the best ranked players ordered by score. */
    std::vector<PlayerData> GetTopPlayers();

    /** Returns the player data associated with an id. */
    PlayerData GetPlayerById(int id);

    /** Returns the player data associated with a username. */
    PlayerData GetPlayerByName(const std::string& username);

    /** Returns the ranking position of a player. */
    int GetPlayerRank(int id);

    /** Updates the score and result counters of a player. */
    bool UpdatePlayerStats(const std::string& username, int pointsToAdd, bool addVictory, bool addDefeat);

private:
    sql::Driver* driver;
    sql::Connection* connection;

    /** Generates a secure password hash using libsodium. */
    std::string HashPassword(const std::string& password);

    /** Checks whether a password matches a stored hash. */
    bool VerifyPassword(const std::string& password, const std::string& storedHash);
};
