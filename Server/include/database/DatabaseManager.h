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
	~DatabaseManager() { disconnectDB(con); };

	bool connectDB(sql::Driver*& driver, sql::Connection*& con);
	void disconnectDB(sql::Connection* con);


	bool registerUserDB(const std::string& name, const std::string& password);
	bool validateLogin(const std::string& name, const std::string& password);

	//función coger top 10
	std::vector <PlayerData> getTop10();

	//Getters
	PlayerData getPlayerbyID(int id);
	int getPlayerRank(int id);

private:
	sql::Driver* driver;
	sql::Connection* con;

};

