#pragma once

#include "DatabaseConfig.h"

#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <vector>
#include <sodium.h> // Para hash

struct PlayerData
{
	int id;
	std::string user;
	int puntuacion_total;
	int victorias;
	int derrotas;
};


class DatabaseManager
{
public:

	DatabaseManager() { driver = nullptr; con = nullptr; };
	~DatabaseManager() { disconnectDB(); };

	bool ConnectDB();
	void disconnectDB();


	bool RegisterUserDB(const std::string& name, const std::string& password);
	bool CheckUserInDB(const std::string& name, const std::string& password);
	bool ValidateLogin(const std::string& name, const std::string& password);

	//función coger top 10
	std::vector <PlayerData> GetTop10();

	//Getters
	PlayerData GetPlayerbyID(int id);
	PlayerData GetPlayerbyName(const std::string& name);
	int GetPlayerRank(int id);

private:
	sql::Driver* driver;
	sql::Connection* con;


	std::string HashPassword(const std::string& password);
	bool VerifyPassword(const std::string& password, const std::string storeHash);
};

