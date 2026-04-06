#pragma once

#include "DatabaseConfig.h"

#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <vector>


struct PlayerData
{
	std::string user;
	int puntuacion_total;
};


class DatabaseManager
{
public:

	DatabaseManager() { driver = nullptr; con = nullptr; };
	~DatabaseManager() { disconnectDB(); };

	bool ConnectDB();
	void disconnectDB();


	bool RegisterUserDB(const std::string& name, const std::string& password);
	bool CheckUserInDB(const std::string& name, std::vector<std::string>& users);
	bool ValidateLogin(const std::string& name, const std::string& password);
	std::vector<std::string> GetAllUsers(sql::Connection* con);

	//función coger top 10
	std::vector <PlayerData> GetTop10();

	//Getters
	PlayerData GetPlayerbyID(int id);
	int GetPlayerRank(int id);

private:
	sql::Driver* driver;
	sql::Connection* con;

};

