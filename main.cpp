#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include "UIManager.h"
#include "Game.h"

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ventana al 88% del escritorio, centrada
    sf::VideoMode escritorio = sf::VideoMode::getDesktopMode();
    unsigned int ancho = static_cast<unsigned int>(escritorio.width  * 0.88f);
    unsigned int alto  = static_cast<unsigned int>(escritorio.height * 0.88f);

    sf::RenderWindow ventana(
        sf::VideoMode(ancho, alto),
        "Snake Arcade - LCP208",
        sf::Style::Titlebar | sf::Style::Close
        );
    ventana.setPosition(sf::Vector2i(
        (escritorio.width  - ancho) / 2,
        (escritorio.height - alto)  / 2
        ));
    ventana.setFramerateLimit(60);

    // Fuente
    sf::Font fuente;
    bool fuenteOk = fuente.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!fuenteOk) fuente.loadFromFile("C:/Windows/Fonts/calibri.ttf");

    // Musica
    // sf::Music hace streaming directo — no carga el archivo completo en RAM
    sf::Music musicaPantallas;
    sf::Music musicaJuego;
    bool hayMusicaPantallas = musicaPantallas.openFromFile("assets/MusicaPantallas.wav");
    bool hayMusicaJuego     = musicaJuego.openFromFile("assets/MusicaJuego.wav");

    musicaPantallas.setLoop(true);
    musicaPantallas.setVolume(60.f);
    musicaJuego.setLoop(true);
    musicaJuego.setVolume(70.f);

    if (hayMusicaPantallas) musicaPantallas.play();

    bool enJuego = false;

    UIManager ui(ventana, fuente);
    sf::Clock reloj;

    //  Bucle principal de la aplicacion
    while (ventana.isOpen()) {
        float dt = reloj.restart().asSeconds();

        sf::Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed)
                ventana.close();
            ui.procesarEventos(evento);
        }

        ui.actualizar(dt);

        if (ui.getPantalla() == Pantalla::SALIR)
            ventana.close();

        // Cambio de musica segun pantalla
        bool ahoraEnJuego = (ui.getPantalla() == Pantalla::JUEGO);
        if (ahoraEnJuego && !enJuego) {
            if (hayMusicaPantallas) musicaPantallas.stop();
            if (hayMusicaJuego)     musicaJuego.play();
            enJuego = true;
        } else if (!ahoraEnJuego && enJuego) {
            if (hayMusicaJuego)     musicaJuego.stop();
            if (hayMusicaPantallas) musicaPantallas.play();
            enJuego = false;
        }

        // Lanzar partida
        if (ui.getPantalla() == Pantalla::JUEGO) {
            Jugador   jug  = ui.getJugadorActivo();
            Nivel     niv  = ui.getNivelSeleccionado();
            ModoJuego modo = ui.getModoSeleccionado();

            // crear y correr el juego — bloquea hasta que el jugador vuelve al menu
            Game juego(ventana, jug, fuente, niv, modo);
            juego.ejecutar();

            // notificar al UIManager para que recargue los datos actualizados
            ui.notificarFinPartida(
                juego.getPuntajeFinal(),
                modo,
                juego.getNivelFinal()
                );

            // volver al menu del jugador al terminar
            ui.setPantalla(Pantalla::MENU_JUEGO);
            ventana.clear(sf::Color(5, 10, 20));
            ventana.display();
        }

        // dibujar UI normal
        ventana.clear(sf::Color(5, 10, 20));
        ui.dibujar();
        ventana.display();
    }

    // detener musica limpiamente al cerrar
    musicaPantallas.stop();
    musicaJuego.stop();

    return 0;
}
