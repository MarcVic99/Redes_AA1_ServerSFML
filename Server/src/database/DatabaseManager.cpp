#include "database/DatabaseManager.h"

bool DatabaseManager::ConnectDB()
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

bool DatabaseManager::RegisterUserDB(const std::string& name, const std::string& password)
{
	if (con == nullptr)
	{
		return false;
	}

	try
	{
		//Select de lo que se ha pasado
		//Comprueba si el usuario ya existe en la base de datos
		if (CheckUserInDB(name, password))
		{
			return false;
		}
		else {
			
			sql::PreparedStatement* stmt = con->prepareStatement(
				"INSERT INTO users (user, password) VALUES ( ?, ?)"
			);

			stmt->setString(1, name);
			stmt->setString(2, HashPassword(password));

			//para saber cuantas rows han sido actualizadas
			int affected_rows = stmt->executeUpdate();	//ejecutamos la query y la guardamos en affected_rows
			std::cout << "number of rows affected: " << affected_rows << std::endl;

			delete stmt;
			return affected_rows > 0;
		}
		
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while creating user: " << e.what() << std::endl;
		return false;
	}
}


//Comprueba si el usuario ya existe en la base de datos
bool DatabaseManager::CheckUserInDB(const std::string& name, const std::string& password)
{
	if (password == "")
	{
		return true;
	}
	try {
		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT user FROM users WHERE user = ?"
		);

		stmt->setString(1, name);
		sql::ResultSet* res = stmt->executeQuery();

		bool exists = res->next();

		delete res;
		delete stmt;

		return exists;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error while creating user: " << e.what() << std::endl;
		return false;
	}
	
}


bool DatabaseManager::ValidateLogin(const std::string& name, const std::string& password)
{
	if (con == nullptr)
	{
		return false;
	}

	try
	{
		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT password FROM users WHERE user = ?"
		);

		stmt->setString(1, name);

		sql::ResultSet* res = stmt->executeQuery();

		if (!res->next())
		{
			delete res;
			delete stmt;
			return false;
		}

		std::string storedHash = res->getString("password");

		delete res;
		delete stmt;

		return VerifyPassword(password, storedHash);
	}
	catch (const sql::SQLException& e)
	{
		std::cout << "Error while validating login: " << e.what() << std::endl;
		return false;
	}
}

std::vector<PlayerData> DatabaseManager::GetTop10()
{
	std::vector<PlayerData> data;

	if (con == nullptr)
		return data;

	try
	{
		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT *"
			"FROM users "
			"ORDER BY puntuacion_total DESC "
			"LIMIT 10"
		);
		sql::ResultSet* res = stmt->executeQuery();

		while (res->next())
		{
			PlayerData p;
			p.user = res->getString("user");
			p.puntuacion_total = res->getInt("puntuacion_total");
			p.victorias = res->getInt("victorias");
			p.derrotas = res->getInt("derrotas");

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

PlayerData DatabaseManager::GetPlayerbyID(int id)
{
	PlayerData data;

	if (con == nullptr)
		return data;

	try
	{

		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT user, puntuacion_total "
			"FROM users WHERE id = ?"
		);

		stmt->setInt(1, id);
		sql::ResultSet* res = stmt->executeQuery();

		if (res->next())
		{
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
		std::cout << "Error getting User by his ID: " << e.what() << std::endl;
	}
	

	return data;
}

PlayerData DatabaseManager::GetPlayerbyName(const std::string& name)
{
	PlayerData data;

	if (con == nullptr)
		return data;

	try
	{
		sql::PreparedStatement* stmt = con->prepareStatement(
			"SELECT id, user FROM users WHERE user = ?"
		);

		stmt->setString(1, name);
		sql::ResultSet* res = stmt->executeQuery();

		if (res->next())
		{
			data.user = res->getString("user");
			data.id = res->getInt("id"); 
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

bool DatabaseManager::UpdatePlayerStats(const std::string& username, int pointsToAdd, bool addVictory, bool addDefeat)
{
	if (con == nullptr)
	{
		return false;
	}

	try
	{
		sql::PreparedStatement* stmt = con->prepareStatement(
			"UPDATE users "
			"SET puntuacion_total = puntuacion_total + ?, "
			"victorias = victorias + ?, "
			"derrotas = derrotas + ? "
			"WHERE user = ?"
		);

		stmt->setInt(1, pointsToAdd);
		stmt->setInt(2, addVictory);
		stmt->setInt(3, addDefeat);
		stmt->setString(4, username);

		int affectedRows = stmt->executeUpdate();

		delete stmt;
		std::cout << "Stats actualizadas para: " << username << " | puntos +" << pointsToAdd << " | victoria: " << addVictory<< " | derrota: " << addDefeat << std::endl;

		return true;
	}
	catch (sql::SQLException e)
	{
		std::cout << "Error updating player stats: " << e.what() << std::endl;
		return false;
	}
}



//-------------------------------------- Funciones de hasheo de password -----------------------------------------

std::string DatabaseManager::HashPassword(const std::string& password)
{
	char hashedPassword[crypto_pwhash_STRBYTES];

	if (crypto_pwhash_str(
		hashedPassword,
		password.c_str(),
		password.size(),
		crypto_pwhash_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
	{
		return "";
	}

	return std::string(hashedPassword);
}


bool DatabaseManager::VerifyPassword(const std::string& password, const std::string storedHash)
{
	return crypto_pwhash_str_verify(
		storedHash.c_str(),
		password.c_str(),
		password.size()) == 0;
}
