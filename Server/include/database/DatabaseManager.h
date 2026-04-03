#pragma once

#include <iostream>
#include "mysql_connection.h"
#include "cppconn/driver.h"

class DatabaseManager
{
public:

	DatabaseManager() {};

	bool connect();
	void disconnect();

	bool registerUserDB(const std::string& name, const std::string& password);

	bool validateLogin(const std::string& name, const std::string& password);

private:
};

