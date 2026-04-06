#include "database/DatabaseManager.h"

bool DatabaseManager::connectDB()
{
	try
	{
		driver = get_driver_instance();
		con = driver->connect(SERVER, USERNAME, PASSWORD);
		con->setSchema(DATABASE);
		
		std::cout << "Connection  done." << std::endl;
		return true;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Could not connect to server. Error message: " << e.what() << std::endl;
	}
    return false;
}

void DatabaseManager::disconnectDB()
{
	if (con != nullptr)
	{
		con->close();

		if (con->isClosed())
		{
			std::cout << "Connection Close" << std::endl;
			delete con;
			con = nullptr;
			driver = nullptr;
		}
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
		if (checkUserInDB(name, password))
		{
			return false;
		}
		else {

			sql::PreparedStatement* stmt = con->prepareStatement(
				"INSERT INTO users (user, password, puntuacion_total, victorias, derrotas) VALUES ( ?, ?, 0, 0, 0)"
			);

			stmt->setString(1, name);
			stmt->setString(2, password);

			//para saber cuantas rows han sido actualizadas
			int affected_rows = stmt->executeUpdate();	//ejecutamos la query y la guardamos en affected_rows
			std::cout << "number of rows affected: " << affected_rows << std::endl;

			delete stmt;
			return true;
		}
		
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while creating user: " << e.what() << std::endl;
		return false;
	}
	return false;
}


bool DatabaseManager::checkUserInDB(const std::string& name, const std::string& password)
{
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
		
		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT user, password FROM users WHERE user = ? AND password = ?"

		);
		sql::ResultSet* res = stmt->executeQuery();

		bool valid = res->next();
		
		delete res;
		delete stmt;

		return valid;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while fetching users: " << e.what() << std::endl;
		return false;
	}
}

std::vector<PlayerData> DatabaseManager::getTop10()
{

	std::vector<PlayerData> data;

	if (con == nullptr)
		return data;

	try
	{

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res =
			stmt->executeQuery(
				"SELECT user, puntuacion_total "
				"FROM users "
				"ORDER BY puntuacion_total DESC "
				"LIMIT 10");

		while (res->next())
		{
			PlayerData p;
			p.user = res->getString("user");
			p.puntuacion_total = res->getInt("puntuacion_total");

			data.push_back(p);
		}
		delete res;
		delete stmt;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error getting Top10 users: " << e.what() << std::endl;
	}


	return data;
}

PlayerData DatabaseManager::getPlayerbyID(int id)
{
	PlayerData data;

	if (con == nullptr)
		return data;

	try
	{

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res =
			stmt->executeQuery(
				"SELECT user, puntuacion_total "
				"FROM users WHERE id = " + std::to_string(id));

		if (res->next())
		{
			data.user = res->getString("user");
			data.puntuacion_total = res->getInt("puntuacion_total");
		}

		delete res;
		delete stmt;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error getting User by his ID: " << e.what() << std::endl;
	}
	

	return data;
}

int DatabaseManager::getPlayerRank(int id)
{
	int rank = -1;

	if (con == nullptr)
		return rank;

	try
	{

		sql::Statement* stmt = con->createStatement();
		sql::ResultSet* res =
			stmt->executeQuery(
				"SELECT COUNT(*) + 1 AS posicion "
				"FROM users "
				"WHERE puntuacion_total > ("
				"SELECT puntuacion_total FROM users WHERE id = " + std::to_string(id) + ")");

		while (res->next())
		{
			rank = res->getInt("posicion");
		}
		delete res;
		delete stmt;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error getting User rank: " << e.what() << std::endl;
	}

	return rank;
}
