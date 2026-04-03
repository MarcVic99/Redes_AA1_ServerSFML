#pragma once

#include <iostream>

class DatabaseManager
{
public:

	DatabaseManager() {};

	bool connect();
	void disconnect();

	bool registerUserDB(const std::string& name, const std::string& password);

private:
};

