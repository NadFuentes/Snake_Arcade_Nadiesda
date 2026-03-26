#include "Archivos.h"
#include <fstream>
#include <sstream>


//  GestorArchivos

// guarda todos los jugadores en el archivo, uno por linea
// formato: nombre|pass|puntNormal|puntComp|nivelMaxComp|partidas|nivelDesbloqueado
bool GestorArchivos::guardarJugadores(const std::vector<Jugador>& jugadores,
                                      const std::string& archivo)
{
    std::ofstream ofs(archivo);
    if (!ofs.is_open()) return false;

    for (const auto& j : jugadores) {
        ofs << j.nombre          << "|"
            << j.password        << "|"
            << j.puntajeNormal   << "|"
            << j.puntajeCompetitivo << "|"
            << j.nivelMaxCompetitivo << "|"
            << j.partidasJugadas << "|"
            << j.nivelDesbloqueado << "\n";
    }
    return true;
}

// lee el archivo y construye la lista — si el archivo no existe devuelve lista vacia
std::vector<Jugador> GestorArchivos::cargarJugadores(const std::string& archivo)
{
    std::vector<Jugador> lista;
    std::ifstream ifs(archivo);
    if (!ifs.is_open()) return lista;

    std::string linea;
    while (std::getline(ifs, linea)) {
        if (linea.empty()) continue;

        // buscar las 6 pipes separadoras
        size_t p1 = linea.find('|');
        size_t p2 = linea.find('|', p1 + 1);
        size_t p3 = linea.find('|', p2 + 1);
        size_t p4 = linea.find('|', p3 + 1);
        size_t p5 = linea.find('|', p4 + 1);
        size_t p6 = linea.find('|', p5 + 1);

        // si le faltan campos es un registro viejo — lo saltamos
        if (p1 == std::string::npos || p2 == std::string::npos ||
            p3 == std::string::npos || p4 == std::string::npos ||
            p5 == std::string::npos || p6 == std::string::npos)
            continue;

        Jugador j;
        j.nombre               = linea.substr(0, p1);
        j.password             = linea.substr(p1 + 1, p2 - p1 - 1);
        j.puntajeNormal        = std::stoi(linea.substr(p2 + 1, p3 - p2 - 1));
        j.puntajeCompetitivo   = std::stoi(linea.substr(p3 + 1, p4 - p3 - 1));
        j.nivelMaxCompetitivo  = std::stoi(linea.substr(p4 + 1, p5 - p4 - 1));
        j.partidasJugadas      = std::stoi(linea.substr(p5 + 1, p6 - p5 - 1));
        j.nivelDesbloqueado    = std::stoi(linea.substr(p6 + 1));
        lista.push_back(j);
    }
    return lista;
}

// simple busqueda por nombre
bool GestorArchivos::existeJugador(const std::string& nombre,
                                   const std::vector<Jugador>& jugadores)
{
    for (const auto& j : jugadores)
        if (j.nombre == nombre) return true;
    return false;
}

// agrega el jugador si no existe y guarda de una vez
bool GestorArchivos::agregarJugador(const Jugador& nuevo,
                                    std::vector<Jugador>& jugadores,
                                    const std::string& archivo)
{
    if (existeJugador(nuevo.nombre, jugadores)) return false;
    jugadores.push_back(nuevo);
    return guardarJugadores(jugadores, archivo);
}

// login basico — compara nombre y password
Jugador* GestorArchivos::validarLogin(const std::string& nombre,
                                      const std::string& pass,
                                      std::vector<Jugador>& jugadores)
{
    for (auto& j : jugadores)
        if (j.nombre == nombre && j.password == pass) return &j;
    return nullptr;
}

// actualiza el puntaje en el modo correspondiente
// si el nuevo puntaje no supera el actual, no hace nada — guardamos el mejor
bool GestorArchivos::actualizarPuntaje(const std::string& nombre,
                                       int nuevoPuntaje,
                                       ModoJuego modo,
                                       int nivelAlcanzado,
                                       std::vector<Jugador>& jugadores,
                                       const std::string& archivo)
{
    for (auto& j : jugadores) {
        if (j.nombre != nombre) continue;

        j.partidasJugadas++;

        if (modo == ModoJuego::NORMAL) {
            // solo reemplazamos si mejoro
            if (nuevoPuntaje > j.puntajeNormal)
                j.puntajeNormal = nuevoPuntaje;

        } else {
            // en competitivo guardamos el mayor puntaje de todas sus partidas
            if (nuevoPuntaje > j.puntajeCompetitivo)
                j.puntajeCompetitivo = nuevoPuntaje;

            // actualizamos el nivel mas alto que llego
            if (nivelAlcanzado > j.nivelMaxCompetitivo)
                j.nivelMaxCompetitivo = nivelAlcanzado;
        }

        return guardarJugadores(jugadores, archivo);
    }
    return false;
}

// desbloquea el siguiente nivel competitivo si es mayor al que tiene
bool GestorArchivos::desbloquearNivel(const std::string& nombre,
                                      int nuevoNivel,
                                      std::vector<Jugador>& jugadores,
                                      const std::string& archivo)
{
    for (auto& j : jugadores) {
        if (j.nombre != nombre) continue;
        if (nuevoNivel > j.nivelDesbloqueado) {
            j.nivelDesbloqueado = nuevoNivel;
            return guardarJugadores(jugadores, archivo);
        }
        return true; // ya tenia ese nivel o mayor, no hay que guardar
    }
    return false;
}

// ordenamiento burbuja descendente
// segun el modo, compara el campo correspondiente
void ordenarPorPuntaje(std::vector<Jugador>& jugadores, ModoJuego modo)
{
    int n = static_cast<int>(jugadores.size());
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            int a = (modo == ModoJuego::NORMAL)
            ? jugadores[j].puntajeNormal
            : jugadores[j].puntajeCompetitivo;
            int b = (modo == ModoJuego::NORMAL)
                        ? jugadores[j+1].puntajeNormal
                        : jugadores[j+1].puntajeCompetitivo;
            if (a < b)
                std::swap(jugadores[j], jugadores[j+1]);
        }
    }
}
