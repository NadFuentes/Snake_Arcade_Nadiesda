#ifndef UIMANAGER_H
#define UIMANAGER_H
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Entities.h"
#include "Archivos.h"

// UIManager: gestiona todas las pantallas del juego
class UIManager {
public:
    UIManager(sf::RenderWindow& ventana, sf::Font& fuente);

    void procesarEventos(sf::Event& evento);
    void actualizar(float dt);
    void dibujar();

    Pantalla  getPantalla()           const { return pantallaActual; }
    void      setPantalla(Pantalla p)       { pantallaActual = p; opcionSel = 0; }
    Jugador   getJugadorActivo()      const { return jugadorActivo; }
    Nivel     getNivelSeleccionado()  const { return nivelSeleccionado; }
    ModoJuego getModoSeleccionado()   const { return modoSeleccionado; }

    // se llama desde main despues de que termina una partida
    void notificarFinPartida(int puntaje, ModoJuego modo, int nivelAlcanzado);

private:
    sf::RenderWindow& ventana;

    Pantalla   pantallaActual;
    Jugador    jugadorActivo;
    Nivel      nivelSeleccionado;
    ModoJuego  modoSeleccionado;
    std::vector<Jugador> listaJugadores;

    sf::Font& fuente;

    // particulas de fondo — lluvia de rayitas
    struct Particula {
        sf::RectangleShape shape;
        float velocidad;
    };
    std::vector<Particula> particulas;
    void initParticulas();
    void updateParticulas(float dt);
    void drawParticulas();

    float tiempo;
    float timerParpadeo;
    bool  cursorVisible;

    // campos de texto
    std::string inputUser;
    std::string inputPass;
    std::string inputPass2;
    int         campoActivo;
    std::string msgError;
    float       timerError;

    int opcionSel;

    // colores
    sf::Color VERDE    = sf::Color(0,   230, 80);
    sf::Color AZUL     = sf::Color(0,   170, 255);
    sf::Color AMARILLO = sf::Color(255, 215, 0);
    sf::Color ROJO     = sf::Color(255, 70,  70);
    sf::Color FONDO    = sf::Color(5,   10,  20);
    sf::Color PANEL    = sf::Color(8,   25,  18, 235);
    sf::Color MORADO   = sf::Color(180, 80,  255); // color para creditos

    // helpers de dibujo
    void drawFondo();
    void drawPanel(float x, float y, float w, float h, sf::Color borde);
    sf::Text makeText(const std::string& str, unsigned sz,
                      sf::Color col, float x, float y, bool center = true);
    void drawBoton(const std::string& label, float cx, float cy,
                   float w, float h, bool sel,
                   sf::Color colorSel = sf::Color(0,230,80));
    void drawCampo(const std::string& label, const std::string& valor,
                   float cx, float cy, float w, bool activo, bool esPass = false);

    // ---- pantallas ----
    void drawInicio();      void eventInicio(sf::Event& e);
    void drawMenu();        void eventMenu(sf::Event& e);
    void drawLogin();       void eventLogin(sf::Event& e);
    void drawCrear();       void eventCrear(sf::Event& e);
    void drawMenuJuego();   void eventMenuJuego(sf::Event& e);
    void drawSelModo();     void eventSelModo(sf::Event& e);
    void drawSelNivel();    void eventSelNivel(sf::Event& e);
    void drawRanking();     void eventRanking(sf::Event& e);
    void drawCreditos();    void eventCreditos(sf::Event& e);  // NUEVO

    void resetCampos();
    float W() const { return static_cast<float>(ventana.getSize().x); }
    float H() const { return static_cast<float>(ventana.getSize().y); }
};

#endif // UIMANAGER_H
