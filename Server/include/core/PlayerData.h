#pragma once
#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <vector>

class PlayerData
{
private:

	std::string user;
	int puntuacion_total;
	int victorias;
	int derrotas;

public:

	//si pierde points negativo
	void AddPuntuacion(int points) { puntuacion_total + points; }
	void AddVictoria() { victorias + 1; }
	void AddDerrota() { derrotas + 1; }


	//getters
	std::string GetUser() { return user; }
	int GetPuntuacion() { return puntuacion_total; }
	int GetVictorias() { return victorias; }
	int GetDerrotas() { return derrotas; }


	//setters
	void SetPuntuacion(int puntuacion) { puntuacion_total = puntuacion; }
	void SetUser(std::string name) { user = name; }
	void SetVictorias(int vict) { victorias = vict; }
	void SetDerrotas(int der) { derrotas = der; }



};

