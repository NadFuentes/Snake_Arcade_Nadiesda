#ifndef ARCHIVOS_H
#define ARCHIVOS_H
#include "Entities.h"
#include <vector>
#include <string>

// manejo de archivos con fstream — aqui se guardan y leen los jugadores

class GestorArchivos {
public:
    // guarda toda la lista en el archivo (sobreescribe todo)
    static bool guardarJugadores(const std::vector<Jugador>& jugadores,
                                 const std::string& archivo = "jugadores.dat");

    // carga la lista desde el archivo al arrancar
    static std::vector<Jugador> cargarJugadores(
        const std::string& archivo = "jugadores.dat");

    // revisa si ya existe un jugador con ese nombre
    static bool existeJugador(const std::string& nombre,
                              const std::vector<Jugador>& jugadores);

    // agrega un jugador nuevo y guarda
    static bool agregarJugador(const Jugador& nuevo,
                               std::vector<Jugador>& jugadores,
                               const std::string& archivo = "jugadores.dat");

    // devuelve puntero al jugador si nombre+pass coinciden, nullptr si no
    static Jugador* validarLogin(const std::string& nombre,
                                 const std::string& pass,
                                 std::vector<Jugador>& jugadores);

    // actualiza el puntaje del jugador segun el modo que jugo
    // solo reemplaza si el nuevo puntaje es mayor al que tenia
    static bool actualizarPuntaje(const std::string& nombre,
                                  int nuevoPuntaje,
                                  ModoJuego modo,
                                  int nivelAlcanzado,
                                  std::vector<Jugador>& jugadores,
                                  const std::string& archivo = "jugadores.dat");

    // desbloquea el siguiente nivel en competitivo si aplica
    static bool desbloquearNivel(const std::string& nombre,
                                 int nuevoNivel,
                                 std::vector<Jugador>& jugadores,
                                 const std::string& archivo = "jugadores.dat");
};

// ordenamiento burbuja descendente
// ordena por puntaje segun el modo indicado
void ordenarPorPuntaje(std::vector<Jugador>& jugadores, ModoJuego modo);

#endif // ARCHIVOS_H
