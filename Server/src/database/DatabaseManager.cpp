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
	con->close();

	if (con->isClosed())
	{
		std::cout << "Connection Close" << std::endl;
		delete con;
		con = nullptr;
	}
}

bool DatabaseManager::registerUserDB(const std::string& name, const std::string& password)
{

	if (con == nullptr)
	{
		return false;
	}

	try
	{

		//Select de lo que se ha pasado

		//Hacer un if(si existe)

		//Si no existe---
	//else {
		sql::Statement* stmt = con->createStatement();
		std::string query = "INSERT INTO users (user, password) VALUES ('" + name + "', '" + password + "')";

		//para saber cuantas rows han sido actualizadas
		int affected_rows = stmt->executeUpdate(query);	//ejecutamos la query y la guardamos en affected_rows
		std::cout << "number of rows affected: " << affected_rows << std::endl;

		delete stmt;
		return true;
	//}
		
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while creating user: " << e.what() << std::endl;
		return false;
	}
	return false;
}

bool DatabaseManager::validateLogin(const std::string& name, const std::string& password)
{

	if (con == nullptr)
	{
		return false;
	}
	try
	{
		
		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res = 
			stmt->executeQuery("SELECT user, password FROM users WHERE user = '" + name + "' AND password = '" + password + "'");

		while (res->next())
		{
			std::cout << res->getString("user") << std::endl;
		}
		delete res;
		delete stmt;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while fetching users: " << e.what() << std::endl;
	}
}
