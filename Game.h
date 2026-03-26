#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include "Entities.h"


//  Game — maneja los 3 niveles y los 2 modos de juego
//  Motor basado en lista de coordenadas (deque + punteros)
class Game {
public:
    // recibe tambien el modo para saber si es normal o competitivo
    Game(sf::RenderWindow& ventana, Jugador& jugador,
         sf::Font& fuente, Nivel nivel, ModoJuego modo);

    void ejecutar();

    // cuantas cerezas lleva en esta sesion competitiva (lo lee UIManager)
    int getCerezasTotal() const { return cerezasTotalesCompetitivo; }

    // nivel al que llego al terminar
    int getNivelFinal() const { return static_cast<int>(nivelActual); }

    // puntaje final de la partida
    int getPuntajeFinal() const { return puntaje; }

private:
    // referencias que vienen de main — no los poseemos
    sf::RenderWindow& ventana;
    Jugador&          jugador;
    sf::Font&         fuente;

    // nivel y modo actuales
    Nivel    nivelActual;
    ModoJuego modoActual;

    // dimensiones del tablero
    static const int CELDAS_X = 30;
    static const int CELDAS_Y = 22;
    int tamCelda;
    int offsetX;
    int offsetY;

    // muros perimetrales del nivel 2 y obstaculos internos del nivel 3
    std::vector<Coordenada> muros;

    // el cuerpo de la serpiente — usamos deque para agregar al frente y quitar atras
    std::deque<Coordenada> cuerpo;
    Coordenada* pCabeza;  // puntero directo a la cabeza
    Coordenada* pCola;    // puntero directo a la cola

    Direccion dirActual;
    Direccion dirBuffer;  // guarda la direccion presionada antes del siguiente tick

    // cereza principal y su tipo (normal o dorada)
    Coordenada  cereza;
    TipoCereza  tipoCereza;
    float       animCereza;

    // contadores de la partida
    EstadoJuego estado;
    int         puntaje;
    int         cerezasComidas;          // cerezas en este nivel/sesion
    int         cerezasTotalesCompetitivo; // acumulado de toda la partida competitiva
    bool        salirAlMenu;

    // para modo competitivo — cuando llega a 25 cerezas sube de nivel
    static const int CEREZAS_PARA_SUBIR = 25;

    // flag que se activa cuando el juego va a subir de nivel (muestra pantalla de transicion)
    bool subiendoNivel;
    float timerTransicion;  // cuanto tiempo lleva mostrando la pantalla de subida

    // tiempo de movimiento
    float timerMovimiento;
    float intervaloMovimiento;
    float tiempoTotal;

    // sistema de particulas para cuando come o muere
    struct Particula {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float        vida;
        sf::Color    color;
    };
    std::vector<Particula> particulas;

    // metodos de logica
    void inicializar();
    void inicializarNivel();   // configura solo lo que cambia al subir de nivel
    void procesarEventos();
    void actualizar(float dt);
    void dibujar();

    void moverSerpiente();
    void ajustarVelocidad();
    bool esCuerpo(const Coordenada& pos) const;
    bool esMuro(const Coordenada& pos) const;
    void generarCereza();
    void generarObstaculosNivel3();  // obstaculos internos aleatorios

    // pantallas especiales
    void dibujarTransicionNivel();  // pantalla "NIVEL X DESBLOQUEADO"
    void dibujarFondo();
    void dibujarTablero();
    void dibujarMuros();
    void dibujarSerpiente();
    void dibujarCabeza(const Coordenada& pos);
    void dibujarSegmento(const Coordenada& pos, int indice);
    void dibujarCereza();
    void dibujarHUD();
    void dibujarGameOver();
    void dibujarParticulas();

    void spawnParticulas(const Coordenada& pos);
    void updateParticulas(float dt);

    sf::Vector2f celdaAPx(const Coordenada& c) const;
    sf::Text     makeText(const std::string& s, unsigned sz,
                      sf::Color col, float x, float y,
                      bool center = true);

    // colores del juego
    static const sf::Color C_FONDO;
    static const sf::Color C_TABLERO;
    static const sf::Color C_GRID;
    static const sf::Color C_SNAKE_CAB;
    static const sf::Color C_SNAKE_A;
    static const sf::Color C_SNAKE_B;
    static const sf::Color C_CEREZA;
    static const sf::Color C_TALLO;
    static const sf::Color C_VERDE;
    static const sf::Color C_AZUL;
    static const sf::Color C_AMARILLO;
    static const sf::Color C_ROJO;
    static const sf::Color C_MURO;
    static const sf::Color C_DORADO;   // cereza dorada nivel 3
    static const sf::Color C_OBSTACULO; // obstaculos nivel 3
};

#endif // GAME_H
