#ifndef ENTITIES_H
#define ENTITIES_H
#include <string>

enum class Nivel { NINGUNO = 0, UNO = 1, DOS = 2, TRES = 3 };
enum class ModoJuego { NORMAL, COMPETITIVO };

struct Jugador {
    std::string nombre;
    std::string password;
    int puntajeNormal;
    int puntajeCompetitivo;
    int nivelMaxCompetitivo;
    int partidasJugadas;
    int nivelDesbloqueado;

    Jugador()
        : nombre(""), password(""),
        puntajeNormal(0), puntajeCompetitivo(0),
        nivelMaxCompetitivo(1), partidasJugadas(0),
        nivelDesbloqueado(1) {}

    Jugador(const std::string& n, const std::string& p)
        : nombre(n), password(p),
        puntajeNormal(0), puntajeCompetitivo(0),
        nivelMaxCompetitivo(1), partidasJugadas(0),
        nivelDesbloqueado(1) {}
};

// pantallas del juego
enum class Pantalla {
    INICIO,
    MENU_PRINCIPAL,
    LOGIN,
    CREAR_USUARIO,
    MENU_JUEGO,
    SELECCION_MODO,
    SELECCION_NIVEL,
    RANKING,
    CREDITOS,
    JUEGO,
    SALIR
};

struct Coordenada {
    int x, y;
    Coordenada(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Coordenada& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Coordenada& o) const { return !(*this == o); }
};

enum class Direccion    { ARRIBA, ABAJO, IZQUIERDA, DERECHA };
enum class EstadoJuego  { JUGANDO, PAUSADO, GAME_OVER, GANADO };
enum class TipoCereza   { NORMAL, DORADA };

#endif // ENTITIES_H
