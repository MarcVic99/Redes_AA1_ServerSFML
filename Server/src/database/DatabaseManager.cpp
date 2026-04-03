#include "database/DatabaseManager.h"

bool DatabaseManager::connectDB(sql::Driver*& driver, sql::Connection*& con)
{
	try
	{
		driver = get_driver_instance();
		con = driver->connect(SERVER, USERNAME, PASSWORD);
		std::cout << "Connection  done." << std::endl;


	}
	catch (sql::SQLException e)
	{
		std::cout << "Could not connect to server. Error message: " << e.what() << std::endl;
	}
    return false;
}

void DatabaseManager::disconnectDB(sql::Connection* con)
{

}

bool DatabaseManager::registerUserDB(const std::string& name, const std::string& password)
{
    return false;
}

bool DatabaseManager::validateLogin(const std::string& name, const std::string& password)
{
    return false;
}
