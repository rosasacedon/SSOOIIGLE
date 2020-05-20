#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <sstream> 
#include <vector>
#include <fstream>
#include <math.h>
#include <queue>
#include <functional> 

#define ERROR -1
#define MAX_DATOS 2048

/***************************************************** ESTRUCTURA PARA RESULTADOS *****************************************************/

struct MsgResultados{

	int numeroHilo;
	int marcadorInicio;
	int marcadorFin;
	int linea;
	std::string palabra;
	std::string palabraPosterior;
    	std::string palabraAnterior;

	MsgResultados(int Hilo, int Inicio, int Fin, int lin, std::string pal, std::string palPost, std::string palAnt):numeroHilo(Hilo), marcadorInicio(Inicio), marcadorFin(Fin), linea(lin), palabra(pal), palabraPosterior(palPost), palabraAnterior(palAnt){}
	
	int getnumeroHilo(){
		return numeroHilo;
	}	
	friend std::ostream& operator << (std::ostream &strm, const MsgResultados &res) {
		strm << "[Hilo " << res.numeroHilo << " -> ";
		strm << "Inicio: " << res.marcadorInicio << " - Final: " << res.marcadorFin << "] :: ";
		strm << "Linea: " << res.linea << " :: ";
		strm << " ... " << res.palabraAnterior;
		strm << " " << res.palabra << " ";
		strm << res.palabraPosterior << " ...";
		return strm;
	}
	friend bool operator < (MsgResultados const & izquierda, MsgResultados const & derecha) {
		return izquierda.linea < derecha.linea;
	}
};

/***************************************************** DEFINICION DE METODOS *****************************************************/

std::mutex sem;
std::vector <std::vector<std::string>> resultados;
std::vector <std::queue<MsgResultados>> colaResultados;
std::vector <std::thread> vectorHilos;

void creacionHilos (int lineasHilo, int numeroHilos, int numeroLineas, std:: string palabra, std::string fichero);
int metodoLineas(std::string fichero);
void buscar(int identificadorHilo, int marcadorInicio, int marcadorFin, std::string palabra, std::vector<std::string> archivo);
std::vector <std::vector<std::string>> separarLineas(int marcadorInicio, int marcadorFin, std::vector<std::string> archivo);
std::vector<std::string> quitarSignos (std::string fichero);
void mostrarResultados();

/*************************************************************** MAIN ***************************************************************/

int main(int argc, char* argv[]){
	
	int numeroLineas;
	int lineasHilo;

	if(argc != 4){
		std::cout << "Introduce el nombre del fichero, la palabra a encontrar y el numero de hilos requerido" << std::endl;
		return EXIT_FAILURE;
	}

	std::string fichero = "./8-libros-txt/";
    fichero.append(argv[1]);
	std::string palabra = argv[2];
	int numeroHilos = atoi(argv[3]);

	numeroLineas=metodoLineas(fichero);
	lineasHilo=floor(numeroLineas/numeroHilos);

	if(numeroHilos <= 0){
		std::cout << "Introduce en el tercer argumento un valor numérico y mayor que 0" << std::endl;
		return EXIT_FAILURE;		
	}

	if((numeroLineas = metodoLineas(fichero)) == ERROR){
		std::cout << "Archivo no encontrado" << std::endl;
		return EXIT_FAILURE;
	}

	if(numeroHilos > numeroLineas){
		numeroHilos = numeroLineas;
	}
	
	creacionHilos (lineasHilo, numeroHilos, numeroLineas, palabra, fichero);
		
return EXIT_SUCCESS;
}

/********************************************************** CREACION DE LOS HILOS **********************************************************/

void creacionHilos (int lineasHilo, int numeroHilos, int numeroLineas, std:: string palabra, std::string fichero){ 

	int marcadorInicio;
	int marcadorFin = 0;
	std::vector<std::thread> vectorHilos;
	std::vector<std::string> archivo;

	archivo = quitarSignos (fichero);

	/************* Crear primeros hilos *************/

	for(int hilo = 0; hilo < numeroHilos - 1; hilo++){

		marcadorInicio = marcadorFin;
		marcadorFin= (lineasHilo * hilo);

		vectorHilos.push_back(std::thread(buscar, hilo, marcadorInicio, marcadorFin, palabra, archivo));
	}

	/*********** Crear último hilo ***********/

	vectorHilos.push_back(std::thread(buscar, numeroHilos - 1, marcadorFin, numeroLineas, palabra, archivo));
	
	/************* Esperar y lanzar los hilos *************/

	int hiloi = 0;

	for (hiloi = 0; hiloi < numeroHilos; hiloi++){

		vectorHilos.at(hiloi).join(); 
	}

	mostrarResultados ();

}

/***************************************************** LINEAS QUE TIENE EL FICHERO *****************************************************/

int metodoLineas(std::string fichero){	

	int lineas = 0;
	std::ifstream archivo(fichero, std::ifstream::in);
	std::string linea;

	if(!archivo.is_open()){
		return ERROR;
	}

	while(std::getline(archivo,linea,'\n')){

            lineas++;
        }

	archivo.close();
	return lineas;
}

/*************************************************************** BUSQUEDA ***************************************************************/

void buscar (int identificadorHilo, int marcadorInicio, int marcadorFin, std::string palabra, std::vector<std::string> archivo){

	std::string linea;
    	char char_fichero;
    	std::string palabraPosterior;
    	std::string palabraAnterior;
    	int indiceLinea = 0;
    	int indiceResultadoLinea = 0;
    	std::vector <std::vector<std::string>> vectorPalabras;
    	vectorPalabras=separarLineas(marcadorInicio,marcadorFin,archivo);
   	std::queue<MsgResultados> colaHilo;

	for(indiceLinea = 0; indiceLinea < vectorPalabras.size(); indiceLinea++){

        	for(indiceResultadoLinea = 0; indiceResultadoLinea < vectorPalabras.at(indiceLinea).size(); indiceResultadoLinea++){

                	if(vectorPalabras.at(indiceLinea).at(indiceResultadoLinea).compare(palabra) == 0){

                        	if (indiceResultadoLinea + 1 <= vectorPalabras.at(indiceLinea).size()){

                            		palabraPosterior = vectorPalabras.at(indiceLinea).at(indiceResultadoLinea + 1).c_str();

                        	}else{

                            		palabraPosterior.clear();
                        	}

                        	if (indiceResultadoLinea -1 >= 0){

                            		palabraAnterior = vectorPalabras.at(indiceLinea).at(indiceResultadoLinea - 1).c_str();

                        	}else{

                            		palabraAnterior.clear();
                        	}

                            	palabra=vectorPalabras.at(indiceLinea).at(indiceResultadoLinea).c_str();
                        
                        
                        MsgResultados resul(identificadorHilo, marcadorInicio, marcadorFin, indiceLinea + 1 + marcadorInicio, palabra, palabraPosterior, palabraAnterior);
                       
                        colaHilo.push(resul);
                    	}
        	}
    	}
	
	sem.lock();
    	colaResultados.push_back(colaHilo);
	sem.unlock();	
	
}

/******************************************************** SEPARARCION DE LINEAS ********************************************************/

std::vector <std::vector<std::string>> separarLineas(int marcadorInicio, int marcadorFin, std::vector<std::string> archivo){

	std::vector <std::vector<std::string>> vectorpalabras;

	for (int i = marcadorInicio; i < marcadorFin; i++){

		std::istringstream lineas_archivo_string(archivo[i].c_str());
	        std::string token;
	        std::vector <std::string> linea;

	        while(std::getline(lineas_archivo_string,token,' ')){

	            linea.push_back(token);
	        }

	        vectorpalabras.push_back(linea);
	}

	return vectorpalabras;

}

/****************************************************** QUITAR SIGNOS DE PUNTUACION ******************************************************/

std::vector<std::string> quitarSignos (std::string fichero){
	
	std::string linea;
    char char_fichero;
    std::ifstream archivo (fichero, std::ifstream::in);
    int indiceLinea = 0;
    int indiceLineaTratada = 0;
    std::vector<std::string> vectorLineas;

	while(std::getline(archivo, linea, '\n')){

			for(indiceLinea = 0; indiceLinea < linea.size(); indiceLinea++){

		   		 char_fichero = linea.at(indiceLinea);

		   		 if(char_fichero =='.' || char_fichero ==',' || char_fichero ==':' || char_fichero =='!' || char_fichero =='?'){

		        		linea = linea.substr(0, indiceLinea) + linea.substr(indiceLinea+1, linea.size());
		    		  }
			}

			vectorLineas.push_back(linea);

		indiceLineaTratada = 0;
	}	

	return vectorLineas;

}

/*************************************************************** RESULTADOS ***************************************************************/

void mostrarResultados () {

	int indiceHilo = 1;
	int indiceVector = 0;
	int finalizado = 0;

	while (finalizado == 0){

		for(unsigned int i = 1; i < colaResultados.size(); i++){ 

			if(colaResultados[i].front().getnumeroHilo() == indiceHilo && colaResultados.size() > indiceHilo){

				while(!colaResultados[i].empty()){

					std::cout << colaResultados[i].front() << std::endl;
					colaResultados[i].pop();	
				}

				indiceHilo++;

			} else if(colaResultados.size() <= indiceHilo){

				finalizado = 1;
			}
		}
	}
}
