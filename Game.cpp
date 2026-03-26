#include "Game.h"
#include "Archivos.h"
#include <cstdlib>
#include <cmath>
#include <sstream>

//  Colores del juego
const sf::Color Game::C_FONDO      = sf::Color(5,   10,  20);
const sf::Color Game::C_TABLERO    = sf::Color(8,   22,  15);
const sf::Color Game::C_GRID       = sf::Color(0,   50,  25, 120);
const sf::Color Game::C_SNAKE_CAB  = sf::Color(0,   230, 80);
const sf::Color Game::C_SNAKE_A    = sf::Color(0,   180, 60);
const sf::Color Game::C_SNAKE_B    = sf::Color(0,   140, 45);
const sf::Color Game::C_CEREZA     = sf::Color(220, 30,  60);
const sf::Color Game::C_TALLO      = sf::Color(0,   160, 40);
const sf::Color Game::C_VERDE      = sf::Color(0,   230, 80);
const sf::Color Game::C_AZUL       = sf::Color(0,   170, 255);
const sf::Color Game::C_AMARILLO   = sf::Color(255, 215, 0);
const sf::Color Game::C_ROJO       = sf::Color(255, 70,  70);
const sf::Color Game::C_MURO       = sf::Color(80,  130, 200);
const sf::Color Game::C_DORADO     = sf::Color(255, 200, 0);   // color cereza dorada
const sf::Color Game::C_OBSTACULO  = sf::Color(180, 60,  200); // obstaculos nivel 3 en morado


//  Constructor — recibe el modo ademas del nivel
Game::Game(sf::RenderWindow& v, Jugador& j, sf::Font& f, Nivel n, ModoJuego m)
    : ventana(v), jugador(j), fuente(f),
    nivelActual(n), modoActual(m),
    tamCelda(0), offsetX(0), offsetY(0),
    pCabeza(nullptr), pCola(nullptr),
    dirActual(Direccion::DERECHA), dirBuffer(Direccion::DERECHA),
    animCereza(0.f),
    tipoCereza(TipoCereza::NORMAL),
    estado(EstadoJuego::JUGANDO),
    puntaje(0), cerezasComidas(0), cerezasTotalesCompetitivo(0),
    salirAlMenu(false),
    subiendoNivel(false), timerTransicion(0.f),
    timerMovimiento(0.f), intervaloMovimiento(0.f), tiempoTotal(0.f)
{
    // calcular el tamano de celda para que el tablero quepa bien en la ventana
    float areaW = ventana.getSize().x * 0.72f;
    float areaH = ventana.getSize().y * 0.82f;
    tamCelda = static_cast<int>(std::min(areaW / CELDAS_X, areaH / CELDAS_Y));

    int totalW = tamCelda * CELDAS_X;
    int totalH = tamCelda * CELDAS_Y;
    offsetX = (static_cast<int>(ventana.getSize().x) - totalW) / 2;
    offsetY = (static_cast<int>(ventana.getSize().y) - totalH) / 2;

    inicializar();
}


//  Inicializar — resetea todo al empezar o reiniciar

void Game::inicializar()
{
    cuerpo.clear();
    particulas.clear();

    // serpiente inicial de 3 segmentos en el centro
    int cx = CELDAS_X / 2;
    int cy = CELDAS_Y / 2;
    cuerpo.push_back(Coordenada(cx,     cy));
    cuerpo.push_back(Coordenada(cx - 1, cy));
    cuerpo.push_back(Coordenada(cx - 2, cy));

    // punteros directos — los actualizamos cada vez que cambia el deque
    pCabeza = &cuerpo.front();
    pCola   = &cuerpo.back();

    dirActual  = Direccion::DERECHA;
    dirBuffer  = Direccion::DERECHA;

    estado      = EstadoJuego::JUGANDO;
    puntaje     = 0;
    cerezasComidas = 0;
    // cerezasTotalesCompetitivo no se resetea aqui — se acumula entre niveles

    timerMovimiento = 0.f;
    tiempoTotal     = 0.f;
    animCereza      = 0.f;
    subiendoNivel   = false;
    timerTransicion = 0.f;

    inicializarNivel();
}

//  inicializarNivel — configura muros y velocidad segun nivel
//  se llama desde inicializar() y al subir de nivel en competitivo
void Game::inicializarNivel()
{
    muros.clear();

    if (nivelActual == Nivel::UNO) {
        intervaloMovimiento = 0.18f;  // nivel 1: velocidad fija, tranquilo

    } else if (nivelActual == Nivel::DOS) {
        intervaloMovimiento = 0.16f;  // nivel 2: empieza mas rapido y va acelerando

        // muros perimetrales — el jugador no puede atravesarlos
        for (int x = 0; x < CELDAS_X; ++x) {
            muros.push_back(Coordenada(x, 0));
            muros.push_back(Coordenada(x, CELDAS_Y - 1));
        }
        for (int y = 1; y < CELDAS_Y - 1; ++y) {
            muros.push_back(Coordenada(0, y));
            muros.push_back(Coordenada(CELDAS_X - 1, y));
        }

    } else if (nivelActual == Nivel::TRES) {
        intervaloMovimiento = 0.14f;  // nivel 3: bien rapido desde el inicio

        // muros perimetrales igual que nivel 2
        for (int x = 0; x < CELDAS_X; ++x) {
            muros.push_back(Coordenada(x, 0));
            muros.push_back(Coordenada(x, CELDAS_Y - 1));
        }
        for (int y = 1; y < CELDAS_Y - 1; ++y) {
            muros.push_back(Coordenada(0, y));
            muros.push_back(Coordenada(CELDAS_X - 1, y));
        }

        // obstaculos internos adicionales
        generarObstaculosNivel3();
    }

    generarCereza();
    salirAlMenu = false;
}

//  Genera obstaculos aleatorios dentro del tablero para nivel 3
void Game::generarObstaculosNivel3()
{
    // ponemos como 15 obstaculos en posiciones aleatorias
    int cantidad = 15;
    for (int i = 0; i < cantidad; ++i) {
        Coordenada pos;
        int intentos = 0;
        do {
            // zona interior — dejamos margen para no poner cosas en los bordes
            pos.x = 3 + std::rand() % (CELDAS_X - 6);
            pos.y = 3 + std::rand() % (CELDAS_Y - 6);
            intentos++;
        } while ((esCuerpo(pos) || esMuro(pos)) && intentos < 100);

        if (intentos < 100)
            muros.push_back(pos);  // los obstaculos se guardan en el mismo vector de muros
    }
}


//  Loop principal del juego
void Game::ejecutar()
{
    sf::Clock reloj;

    while (ventana.isOpen() && !salirAlMenu) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;  // cap de delta time para evitar saltos raros

        procesarEventos();
        if (salirAlMenu) break;
        actualizar(dt);
        dibujar();
    }
}


//  Procesar eventos de teclado
void Game::procesarEventos()
{
    sf::Event e;
    while (ventana.pollEvent(e)) {
        if (e.type == sf::Event::Closed)
            ventana.close();

        if (e.type == sf::Event::KeyPressed) {

            // movimiento — el buffer evita que se pierdan inputs entre ticks
            if ((e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W) &&
                dirActual != Direccion::ABAJO)
                dirBuffer = Direccion::ARRIBA;

            if ((e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S) &&
                dirActual != Direccion::ARRIBA)
                dirBuffer = Direccion::ABAJO;

            if ((e.key.code == sf::Keyboard::Left || e.key.code == sf::Keyboard::A) &&
                dirActual != Direccion::DERECHA)
                dirBuffer = Direccion::IZQUIERDA;

            if ((e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D) &&
                dirActual != Direccion::IZQUIERDA)
                dirBuffer = Direccion::DERECHA;

            // pausa con P o ESC
            if (e.key.code == sf::Keyboard::P || e.key.code == sf::Keyboard::Escape) {
                if (estado == EstadoJuego::JUGANDO && !subiendoNivel)
                    estado = EstadoJuego::PAUSADO;
                else if (estado == EstadoJuego::PAUSADO)
                    estado = EstadoJuego::JUGANDO;
            }

            // reiniciar desde game over
            if (e.key.code == sf::Keyboard::R && estado == EstadoJuego::GAME_OVER) {
                // en competitivo volver al nivel 1 al reiniciar
                if (modoActual == ModoJuego::COMPETITIVO) {
                    nivelActual = Nivel::UNO;
                    cerezasTotalesCompetitivo = 0;
                }
                inicializar();
            }

            // salir al menu — guarda puntaje antes de irse
            if (e.key.code == sf::Keyboard::M &&
                (estado == EstadoJuego::GAME_OVER || estado == EstadoJuego::PAUSADO)) {

                std::vector<Jugador> lista = GestorArchivos::cargarJugadores();
                GestorArchivos::actualizarPuntaje(
                    jugador.nombre, puntaje, modoActual,
                    static_cast<int>(nivelActual), lista);
                salirAlMenu = true;
            }

            // en modo competitivo, ENTER avanza desde la pantalla de transicion
            if (e.key.code == sf::Keyboard::Return && subiendoNivel) {
                subiendoNivel = false;
                // resetear posicion de serpiente sin borrar el puntaje total
                cuerpo.clear();
                particulas.clear();
                int cx = CELDAS_X / 2;
                int cy = CELDAS_Y / 2;
                cuerpo.push_back(Coordenada(cx,     cy));
                cuerpo.push_back(Coordenada(cx - 1, cy));
                cuerpo.push_back(Coordenada(cx - 2, cy));
                pCabeza = &cuerpo.front();
                pCola   = &cuerpo.back();
                dirActual  = Direccion::DERECHA;
                dirBuffer  = Direccion::DERECHA;
                cerezasComidas = 0;
                timerMovimiento = 0.f;
                tiempoTotal     = 0.f;
                estado = EstadoJuego::JUGANDO;
                inicializarNivel();
            }
        }
    }
}


//  Actualizar logica de juego
void Game::actualizar(float dt)
{
    // si estamos mostrando la transicion de nivel, solo contar el timer
    if (subiendoNivel) {
        timerTransicion += dt;
        return;
    }

    if (estado != EstadoJuego::JUGANDO) {
        updateParticulas(dt);
        return;
    }

    tiempoTotal += dt;
    animCereza  += dt;

    timerMovimiento += dt;
    if (timerMovimiento >= intervaloMovimiento) {
        timerMovimiento = 0.f;
        moverSerpiente();
    }

    updateParticulas(dt);
}

//  moverSerpiente — nucleo del motor, usa punteros
void Game::moverSerpiente()
{
    dirActual = dirBuffer;

    // nueva posicion de cabeza usando el puntero pCabeza
    Coordenada nuevaCabeza = *pCabeza;
    switch (dirActual) {
    case Direccion::ARRIBA:    nuevaCabeza.y -= 1; break;
    case Direccion::ABAJO:     nuevaCabeza.y += 1; break;
    case Direccion::IZQUIERDA: nuevaCabeza.x -= 1; break;
    case Direccion::DERECHA:   nuevaCabeza.x += 1; break;
    }

    // colision con bordes del tablero (nivel 1 muere si sale)
    if (nuevaCabeza.x < 0 || nuevaCabeza.x >= CELDAS_X ||
        nuevaCabeza.y < 0 || nuevaCabeza.y >= CELDAS_Y) {
        spawnParticulas(cuerpo.front());
        estado = EstadoJuego::GAME_OVER;

        // guardar puntaje cuando muere
        std::vector<Jugador> lista = GestorArchivos::cargarJugadores();
        GestorArchivos::actualizarPuntaje(
            jugador.nombre, puntaje, modoActual,
            static_cast<int>(nivelActual), lista);
        return;
    }

    // colision con muros u obstaculos
    if (esMuro(nuevaCabeza)) {
        spawnParticulas(cuerpo.front());
        estado = EstadoJuego::GAME_OVER;

        std::vector<Jugador> lista = GestorArchivos::cargarJugadores();
        GestorArchivos::actualizarPuntaje(
            jugador.nombre, puntaje, modoActual,
            static_cast<int>(nivelActual), lista);
        return;
    }

    // colision consigo mismo — excepto la ultima cola que se va a mover
    for (int i = 0; i < static_cast<int>(cuerpo.size()) - 1; ++i) {
        if (cuerpo[i] == nuevaCabeza) {
            spawnParticulas(cuerpo.front());
            estado = EstadoJuego::GAME_OVER;

            std::vector<Jugador> lista = GestorArchivos::cargarJugadores();
            GestorArchivos::actualizarPuntaje(
                jugador.nombre, puntaje, modoActual,
                static_cast<int>(nivelActual), lista);
            return;
        }
    }

    // verificar si come la cereza
    bool comio = (nuevaCabeza == cereza);

    if (comio) {
        // crece: agrega cabeza sin quitar cola
        cuerpo.push_front(nuevaCabeza);
        pCabeza = &cuerpo.front();
        pCola   = &cuerpo.back();

        cerezasComidas++;
        cerezasTotalesCompetitivo++;

        // dorada vale el doble
        puntaje += (tipoCereza == TipoCereza::DORADA) ? 20 : 10;

        spawnParticulas(cereza);
        ajustarVelocidad();

        // en modo competitivo revisar si llego a las 25 cerezas
        if (modoActual == ModoJuego::COMPETITIVO &&
            cerezasComidas >= CEREZAS_PARA_SUBIR) {

            Nivel siguienteNivel = static_cast<Nivel>(static_cast<int>(nivelActual) + 1);

            if (static_cast<int>(nivelActual) < 3) {
                // guardar el nivel desbloqueado
                std::vector<Jugador> lista = GestorArchivos::cargarJugadores();
                GestorArchivos::desbloquearNivel(
                    jugador.nombre, static_cast<int>(siguienteNivel), lista);

                nivelActual = siguienteNivel;
                subiendoNivel   = true;
                timerTransicion = 0.f;

            } else {
                // ya esta en nivel 3 y completo 25 cerezas — sigue jugando sin fin
                cerezasComidas = 0;
            }
        }

        generarCereza();

    } else {
        // se mueve: agrega cabeza y quita cola
        cuerpo.push_front(nuevaCabeza);
        cuerpo.pop_back();

        // siempre actualizar los punteros despues de modificar el deque
        pCabeza = &cuerpo.front();
        pCola   = &cuerpo.back();
    }
}

//  ajustarVelocidad — solo en nivel 2 y 3 incrementa cada 3 cerezas
void Game::ajustarVelocidad()
{
    if (nivelActual == Nivel::DOS) {
        if (cerezasComidas % 3 == 0) {
            intervaloMovimiento -= 0.01f;
            if (intervaloMovimiento < 0.07f)
                intervaloMovimiento = 0.07f;
        }
    } else if (nivelActual == Nivel::TRES) {
        // nivel 3 acelera un poco mas agresivo
        if (cerezasComidas % 3 == 0) {
            intervaloMovimiento -= 0.012f;
            if (intervaloMovimiento < 0.055f)
                intervaloMovimiento = 0.055f;
        }
    }
}

//  generarCereza — elige posicion libre y decide si es dorada
void Game::generarCereza()
{
    Coordenada pos;
    int intentos = 0;
    do {
        pos.x = std::rand() % CELDAS_X;
        pos.y = std::rand() % CELDAS_Y;
        intentos++;
    } while ((esCuerpo(pos) || esMuro(pos)) && intentos < 200);

    cereza = pos;

    // cereza dorada: solo aparece en nivel 3, con 10% de probabilidad
    if (nivelActual == Nivel::TRES && (std::rand() % 10) == 0)
        tipoCereza = TipoCereza::DORADA;
    else
        tipoCereza = TipoCereza::NORMAL;
}


//  Helpers de colision
bool Game::esCuerpo(const Coordenada& pos) const
{
    for (const auto& c : cuerpo)
        if (c == pos) return true;
    return false;
}

bool Game::esMuro(const Coordenada& pos) const
{
    for (const auto& m : muros)
        if (m == pos) return true;
    return false;
}


//  Particulas — efecto visual al comer o morir
void Game::spawnParticulas(const Coordenada& pos)
{
    sf::Vector2f centro = celdaAPx(pos);
    centro.x += tamCelda / 2.f;
    centro.y += tamCelda / 2.f;

    for (int i = 0; i < 18; ++i) {
        Particula p;
        p.pos  = centro;
        float ang = (std::rand() % 360) * 3.14159f / 180.f;
        float spd = 40.f + std::rand() % 120;
        p.vel  = sf::Vector2f(std::cos(ang)*spd, std::sin(ang)*spd);
        p.vida = 0.6f + (std::rand() % 40) / 100.f;

        if (estado == EstadoJuego::JUGANDO) {
            // particulas doradas si comio cereza dorada
            if (tipoCereza == TipoCereza::DORADA)
                p.color = sf::Color(255, 200 + std::rand()%55, 0);
            else
                p.color = sf::Color(0, 200 + std::rand()%55, 60);
        } else {
            p.color = sf::Color(220 + std::rand()%35, 30 + std::rand()%60, 30);
        }
        particulas.push_back(p);
    }
}

void Game::updateParticulas(float dt)
{
    for (auto& p : particulas) {
        p.pos  += p.vel * dt;
        p.vel  *= 0.92f;
        p.vida -= dt;
    }
    particulas.erase(
        std::remove_if(particulas.begin(), particulas.end(),
                       [](const Particula& p){ return p.vida <= 0.f; }),
        particulas.end());
}


//  Helpers graficos
sf::Vector2f Game::celdaAPx(const Coordenada& c) const
{
    return sf::Vector2f(
        static_cast<float>(offsetX + c.x * tamCelda),
        static_cast<float>(offsetY + c.y * tamCelda)
        );
}

sf::Text Game::makeText(const std::string& s, unsigned sz,
                        sf::Color col, float x, float y, bool center)
{
    sf::Text t;
    t.setFont(fuente);
    t.setString(s);
    t.setCharacterSize(sz);
    t.setFillColor(col);
    if (center) {
        auto b = t.getLocalBounds();
        t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    }
    t.setPosition(x, y);
    return t;
}

//  Dibujar — llama a todo lo demas
void Game::dibujar()
{
    ventana.clear(C_FONDO);

    // si esta mostrando pantalla de transicion de nivel, solo eso
    if (subiendoNivel) {
        dibujarTransicionNivel();
        ventana.display();
        return;
    }

    dibujarFondo();
    dibujarTablero();
    dibujarMuros();
    dibujarCereza();
    dibujarSerpiente();
    dibujarParticulas();
    dibujarHUD();

    if (estado == EstadoJuego::GAME_OVER) dibujarGameOver();

    if (estado == EstadoJuego::PAUSADO) {
        sf::RectangleShape overlay(sf::Vector2f(
            static_cast<float>(ventana.getSize().x),
            static_cast<float>(ventana.getSize().y)));
        overlay.setFillColor(sf::Color(0,0,0,140));
        ventana.draw(overlay);

        float cx = ventana.getSize().x/2.f;
        float cy = ventana.getSize().y/2.f;
        ventana.draw(makeText("PAUSA", 42, C_AMARILLO, cx, cy-30));
        ventana.draw(makeText("[ P / ESC ] Continuar    [ M ] Menu", 18,
                              sf::Color(200,200,200), cx, cy+30));
    }

    ventana.display();
}


//  Pantalla de transicion al subir de nivel en competitivo
void Game::dibujarTransicionNivel()
{
    ventana.clear(C_FONDO);

    float cx = ventana.getSize().x / 2.f;
    float cy = ventana.getSize().y / 2.f;

    // panel central
    sf::RectangleShape panel(sf::Vector2f(580, 320));
    panel.setOrigin(290, 160);
    panel.setPosition(cx, cy);
    panel.setFillColor(sf::Color(8, 22, 15, 245));
    panel.setOutlineThickness(3.f);
    panel.setOutlineColor(C_AMARILLO);
    ventana.draw(panel);

    // titulo parpadeante
    sf::Uint8 pa = static_cast<sf::Uint8>(150 + 105 * std::sin(timerTransicion * 4.f));
    ventana.draw(makeText("NIVEL COMPLETADO!", 36,
                          sf::Color(255, 215, 0, pa), cx, cy - 100));

    // que nivel viene ahora
    std::string txtNivel = "Bienvenido al Nivel " + std::to_string(static_cast<int>(nivelActual));
    ventana.draw(makeText(txtNivel, 28, C_VERDE, cx, cy - 30));

    // descripcion del nuevo nivel
    std::string desc = "";
    if (nivelActual == Nivel::DOS) desc = "Velocidad incremental + Muros perimetrales";
    if (nivelActual == Nivel::TRES) desc = "Obstaculos internos + Cerezas doradas!";
    ventana.draw(makeText(desc, 18, C_AZUL, cx, cy + 30));

    // puntaje acumulado
    std::ostringstream ss;
    ss << "Puntaje acumulado: " << puntaje;
    ventana.draw(makeText(ss.str(), 20, C_AMARILLO, cx, cy + 80));

    // instruccion
    sf::Uint8 ia = static_cast<sf::Uint8>(100 + 100 * std::sin(timerTransicion * 3.f));
    ventana.draw(makeText("[ ENTER ] Continuar", 16,
                          sf::Color(180, 240, 180, ia), cx, cy + 130));
}

//  Dibujar fondo
void Game::dibujarFondo()
{
    sf::RectangleShape bg(sf::Vector2f(
        static_cast<float>(ventana.getSize().x),
        static_cast<float>(ventana.getSize().y)));
    bg.setFillColor(C_FONDO);
    ventana.draw(bg);
}

//  Dibujar tablero con grilla
void Game::dibujarTablero()
{
    float tw = static_cast<float>(tamCelda * CELDAS_X);
    float th = static_cast<float>(tamCelda * CELDAS_Y);
    float ox = static_cast<float>(offsetX);
    float oy = static_cast<float>(offsetY);

    sf::RectangleShape tablero(sf::Vector2f(tw, th));
    tablero.setPosition(ox, oy);
    tablero.setFillColor(C_TABLERO);
    tablero.setOutlineThickness(2.f);

    // borde del tablero cambia de color segun nivel
    sf::Color bordeNivel = C_VERDE;
    if (nivelActual == Nivel::DOS) bordeNivel = C_AZUL;
    if (nivelActual == Nivel::TRES) bordeNivel = C_OBSTACULO;
    tablero.setOutlineColor(bordeNivel);
    ventana.draw(tablero);

    sf::RectangleShape linea;
    linea.setFillColor(C_GRID);
    for (int x = 1; x < CELDAS_X; ++x) {
        linea.setSize(sf::Vector2f(1.f, th));
        linea.setPosition(ox + x * tamCelda, oy);
        ventana.draw(linea);
    }
    for (int y = 1; y < CELDAS_Y; ++y) {
        linea.setSize(sf::Vector2f(tw, 1.f));
        linea.setPosition(ox, oy + y * tamCelda);
        ventana.draw(linea);
    }
}

//  Dibujar muros — nivel 2 son azules, nivel 3 los obstaculos son morados
void Game::dibujarMuros()
{
    if (muros.empty()) return;

    for (int i = 0; i < static_cast<int>(muros.size()); ++i) {
        const auto& m = muros[i];
        sf::Vector2f p = celdaAPx(m);
        float s = static_cast<float>(tamCelda);

        // los primeros muros son perimetrales (azules), los del nivel 3 internos son morados
        bool esPerimetral = (m.x == 0 || m.x == CELDAS_X-1 || m.y == 0 || m.y == CELDAS_Y-1);
        sf::Color colorBloque = esPerimetral ? sf::Color(20, 50, 90) : sf::Color(50, 15, 60);
        sf::Color colorBorde  = esPerimetral ? C_MURO : C_OBSTACULO;

        sf::RectangleShape bloque(sf::Vector2f(s, s));
        bloque.setPosition(p);
        bloque.setFillColor(colorBloque);
        bloque.setOutlineThickness(1.f);
        bloque.setOutlineColor(colorBorde);
        ventana.draw(bloque);

        sf::RectangleShape brillo(sf::Vector2f(s - 4, s - 4));
        brillo.setPosition(p.x + 2, p.y + 2);
        brillo.setFillColor(sf::Color::Transparent);
        brillo.setOutlineThickness(1.f);
        brillo.setOutlineColor(sf::Color(colorBorde.r, colorBorde.g, colorBorde.b, 80));
        ventana.draw(brillo);
    }
}

//  Dibujar serpiente
void Game::dibujarSerpiente()
{
    for (int i = static_cast<int>(cuerpo.size()) - 1; i >= 0; --i) {
        if (i == 0) dibujarCabeza(cuerpo[i]);
        else        dibujarSegmento(cuerpo[i], i);
    }
}

void Game::dibujarCabeza(const Coordenada& pos)
{
    sf::Vector2f p = celdaAPx(pos);
    float s = static_cast<float>(tamCelda);
    float margen = 1.f;

    sf::RectangleShape cab(sf::Vector2f(s - margen*2, s - margen*2));
    cab.setPosition(p.x + margen, p.y + margen);
    cab.setFillColor(C_SNAKE_CAB);

    sf::RectangleShape borde(sf::Vector2f(s - margen*2, s - margen*2));
    borde.setPosition(p.x + margen, p.y + margen);
    borde.setFillColor(sf::Color::Transparent);
    borde.setOutlineThickness(1.5f);
    borde.setOutlineColor(sf::Color(100, 255, 140));
    ventana.draw(cab);
    ventana.draw(borde);

    // ojos que apuntan hacia donde va la serpiente
    float ox1, oy1, ox2, oy2;
    float eyeR  = s * 0.13f;
    float eyeOff = s * 0.25f;
    float eyeFwd = s * 0.68f;

    switch (dirActual) {
    case Direccion::DERECHA:
        ox1 = p.x + eyeFwd; oy1 = p.y + eyeOff;
        ox2 = p.x + eyeFwd; oy2 = p.y + s - eyeOff; break;
    case Direccion::IZQUIERDA:
        ox1 = p.x + s - eyeFwd; oy1 = p.y + eyeOff;
        ox2 = p.x + s - eyeFwd; oy2 = p.y + s - eyeOff; break;
    case Direccion::ARRIBA:
        ox1 = p.x + eyeOff;     oy1 = p.y + s - eyeFwd;
        ox2 = p.x + s - eyeOff; oy2 = p.y + s - eyeFwd; break;
    case Direccion::ABAJO:
    default:
        ox1 = p.x + eyeOff;     oy1 = p.y + eyeFwd;
        ox2 = p.x + s - eyeOff; oy2 = p.y + eyeFwd; break;
    }

    sf::CircleShape ojo(eyeR);
    ojo.setFillColor(sf::Color::White);
    ojo.setOrigin(eyeR, eyeR);
    ojo.setPosition(ox1, oy1); ventana.draw(ojo);
    ojo.setPosition(ox2, oy2); ventana.draw(ojo);

    sf::CircleShape pupila(eyeR * 0.5f);
    pupila.setFillColor(sf::Color(10, 10, 10));
    pupila.setOrigin(eyeR * 0.5f, eyeR * 0.5f);
    pupila.setPosition(ox1, oy1); ventana.draw(pupila);
    pupila.setPosition(ox2, oy2); ventana.draw(pupila);
}

void Game::dibujarSegmento(const Coordenada& pos, int indice)
{
    sf::Vector2f p = celdaAPx(pos);
    float s  = static_cast<float>(tamCelda);
    float mg = 2.f;

    sf::Color color = (indice % 2 == 0) ? C_SNAKE_A : C_SNAKE_B;

    sf::RectangleShape seg(sf::Vector2f(s - mg*2, s - mg*2));
    seg.setPosition(p.x + mg, p.y + mg);
    seg.setFillColor(color);
    ventana.draw(seg);

    sf::RectangleShape detalle(sf::Vector2f(s - mg*2 - 4, s - mg*2 - 4));
    detalle.setPosition(p.x + mg + 2, p.y + mg + 2);
    detalle.setFillColor(sf::Color::Transparent);
    detalle.setOutlineThickness(1.f);
    detalle.setOutlineColor(sf::Color(color.r, color.g, color.b, 80));
    ventana.draw(detalle);
}

//  Dibujar cereza — dorada si es especial
void Game::dibujarCereza()
{
    sf::Vector2f p = celdaAPx(cereza);
    float s  = static_cast<float>(tamCelda);
    float cx = p.x + s / 2.f;
    float cy = p.y + s / 2.f;

    // pulso diferente si es dorada — late mas rapido
    float velPulso = (tipoCereza == TipoCereza::DORADA) ? 7.f : 4.f;
    float pulso    = 0.85f + 0.15f * std::sin(animCereza * velPulso);
    float radio    = (s * 0.36f) * pulso;

    sf::Color colorCereza = (tipoCereza == TipoCereza::DORADA) ? C_DORADO : C_CEREZA;
    sf::Color colorHalo   = (tipoCereza == TipoCereza::DORADA)
                              ? sf::Color(255, 200, 0, 50)
                              : sf::Color(220, 30, 60, 40);

    // halo exterior
    sf::CircleShape halo(radio + 5.f);
    halo.setOrigin(radio + 5.f, radio + 5.f);
    halo.setPosition(cx, cy + s * 0.08f);
    halo.setFillColor(colorHalo);
    ventana.draw(halo);

    // cuerpo de la cereza
    sf::CircleShape circulo(radio);
    circulo.setOrigin(radio, radio);
    circulo.setPosition(cx, cy + s * 0.08f);
    circulo.setFillColor(colorCereza);
    circulo.setOutlineThickness(1.5f);
    circulo.setOutlineColor(
        tipoCereza == TipoCereza::DORADA
            ? sf::Color(255, 240, 120)
            : sf::Color(255, 80, 100));
    ventana.draw(circulo);

    // brillo interno
    sf::CircleShape brillo(radio * 0.3f);
    brillo.setOrigin(radio * 0.3f, radio * 0.3f);
    brillo.setPosition(cx - radio * 0.25f, cy + s * 0.08f - radio * 0.25f);
    brillo.setFillColor(sf::Color(255, 255, 255, 180));
    ventana.draw(brillo);

    // tallo
    sf::RectangleShape tallo(sf::Vector2f(2.f, s * 0.28f));
    tallo.setOrigin(1.f, s * 0.28f);
    tallo.setPosition(cx, cy - radio * 0.6f + s * 0.08f);
    tallo.setFillColor(C_TALLO);
    tallo.setRotation(-15.f);
    ventana.draw(tallo);

    // hojita
    sf::CircleShape hoja(s * 0.09f, 3);
    hoja.setOrigin(s * 0.09f, s * 0.09f);
    hoja.setPosition(cx + 3.f, cy - radio * 0.55f + s * 0.08f - s * 0.12f);
    hoja.setFillColor(C_TALLO);
    ventana.draw(hoja);

    // etiqueta "x2" si es dorada
    if (tipoCereza == TipoCereza::DORADA) {
        auto txt = makeText("x2", 11, sf::Color(255, 240, 0),
                            cx + radio + 4.f, cy - radio, false);
        ventana.draw(txt);
    }
}

//  HUD — info en pantalla durante el juego
void Game::dibujarHUD()
{
    float W  = static_cast<float>(ventana.getSize().x);

    // puntaje arriba a la izquierda
    std::ostringstream ss;
    ss << "PUNTAJE: " << puntaje;
    ventana.draw(makeText(ss.str(), 20, C_AMARILLO, 20.f, 14.f, false));

    // nombre del jugador
    ventana.draw(makeText("JUGADOR: " + jugador.nombre, 16, C_VERDE, 20.f, 42.f, false));

    // modo de juego
    std::string txtModo = (modoActual == ModoJuego::COMPETITIVO) ? "COMPETITIVO" : "NORMAL";
    ventana.draw(makeText("MODO: " + txtModo, 14,
                          modoActual == ModoJuego::COMPETITIVO ? C_AMARILLO : sf::Color(160,160,160),
                          20.f, 66.f, false));

    // cerezas a la derecha
    std::ostringstream sc;
    if (modoActual == ModoJuego::COMPETITIVO) {
        // en competitivo muestra cuantas lleva para subir de nivel
        sc << "CEREZAS: " << cerezasComidas << " / " << CEREZAS_PARA_SUBIR;
    } else {
        sc << "CEREZAS: " << cerezasComidas;
    }
    ventana.draw(makeText(sc.str(), 16, C_ROJO, W - 220.f, 14.f, false));

    // nombre del nivel
    std::string nombreNivel;
    if (nivelActual == Nivel::UNO)  nombreNivel = "NIVEL 1 - CLASICO";
    if (nivelActual == Nivel::DOS)  nombreNivel = "NIVEL 2 - AVANZADO";
    if (nivelActual == Nivel::TRES) nombreNivel = "NIVEL 3 - EXTREMO";
    ventana.draw(makeText(nombreNivel, 16, C_AZUL, W - 220.f, 38.f, false));

    // longitud de la serpiente centrado
    std::ostringstream sl;
    sl << "LONGITUD: " << cuerpo.size();
    ventana.draw(makeText(sl.str(), 15, sf::Color(160,220,160), W / 2.f, 16.f, true));

    // barra de progreso de nivel en competitivo
    if (modoActual == ModoJuego::COMPETITIVO) {
        float progreso = static_cast<float>(cerezasComidas) / CEREZAS_PARA_SUBIR;
        if (progreso > 1.f) progreso = 1.f;

        float barW = 200.f;
        float barX = W / 2.f - barW / 2.f;
        float barY = static_cast<float>(offsetY) + CELDAS_Y * tamCelda + 20.f;

        ventana.draw(makeText("PROGRESO NIVEL", 11, C_AMARILLO, barX - 10.f, barY + 5.f, false));

        sf::RectangleShape barBase(sf::Vector2f(barW, 10.f));
        barBase.setPosition(barX + 105.f, barY);
        barBase.setFillColor(sf::Color(0, 30, 10));
        barBase.setOutlineThickness(1.f);
        barBase.setOutlineColor(C_AMARILLO);
        ventana.draw(barBase);

        sf::RectangleShape barFill(sf::Vector2f(barW * progreso, 10.f));
        barFill.setPosition(barX + 105.f, barY);
        sf::Uint8 r = static_cast<sf::Uint8>(progreso * 255);
        sf::Uint8 g = static_cast<sf::Uint8>((1.f - progreso) * 200 + 50);
        barFill.setFillColor(sf::Color(r, g, 0));
        ventana.draw(barFill);
    }

    // barra de velocidad en nivel 2 normal
    if (nivelActual == Nivel::DOS && modoActual == ModoJuego::NORMAL) {
        float velPct = 1.f - (intervaloMovimiento - 0.07f) / (0.16f - 0.07f);
        if (velPct < 0.f) velPct = 0.f;
        if (velPct > 1.f) velPct = 1.f;

        float barW = 160.f;
        float barX = W / 2.f - barW / 2.f;
        float barY = static_cast<float>(offsetY) + CELDAS_Y * tamCelda + 20.f;

        ventana.draw(makeText("VELOCIDAD", 11, C_AZUL, barX - 10.f, barY + 5.f, false));

        sf::RectangleShape barBase(sf::Vector2f(barW, 10.f));
        barBase.setPosition(barX + 85.f, barY);
        barBase.setFillColor(sf::Color(0, 30, 60));
        barBase.setOutlineThickness(1.f);
        barBase.setOutlineColor(C_AZUL);
        ventana.draw(barBase);

        sf::RectangleShape barFill(sf::Vector2f(barW * velPct, 10.f));
        barFill.setPosition(barX + 85.f, barY);
        sf::Uint8 r = static_cast<sf::Uint8>(velPct * 255);
        sf::Uint8 g = static_cast<sf::Uint8>((1.f - velPct) * 200);
        barFill.setFillColor(sf::Color(r, g, 80));
        ventana.draw(barFill);
    }

    // controles abajo del tablero
    float yBot = static_cast<float>(offsetY) + CELDAS_Y * tamCelda + 48.f;
    ventana.draw(makeText("FLECHAS / WASD: Mover    P / ESC: Pausa    M: Menu",
                          13, sf::Color(80,150,80,200), W/2.f, yBot, true));
}


//  Game Over
void Game::dibujarGameOver()
{
    sf::RectangleShape overlay(sf::Vector2f(
        static_cast<float>(ventana.getSize().x),
        static_cast<float>(ventana.getSize().y)));
    overlay.setFillColor(sf::Color(0,0,0,160));
    ventana.draw(overlay);

    float cx = ventana.getSize().x / 2.f;
    float cy = ventana.getSize().y / 2.f;

    sf::RectangleShape panel(sf::Vector2f(520, 300));
    panel.setOrigin(260, 150);
    panel.setPosition(cx, cy);
    panel.setFillColor(sf::Color(8, 18, 12, 240));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(C_ROJO);
    ventana.draw(panel);

    ventana.draw(makeText("GAME OVER", 40, C_ROJO, cx, cy - 90));
    ventana.draw(makeText("PUNTAJE FINAL", 18, C_AMARILLO, cx, cy - 30));

    std::ostringstream ss;
    ss << puntaje << " puntos  |  " << cerezasComidas << " cerezas";
    ventana.draw(makeText(ss.str(), 20, sf::Color::White, cx, cy + 16));

    // en competitivo mostrar el nivel al que llego
    if (modoActual == ModoJuego::COMPETITIVO) {
        std::string lvlTxt = "Nivel alcanzado: " + std::to_string(static_cast<int>(nivelActual));
        ventana.draw(makeText(lvlTxt, 16, C_AZUL, cx, cy + 50));
    }

    ventana.draw(makeText("[ R ] Reintentar    [ M ] Menu", 16,
                          sf::Color(180,240,180), cx, cy + 88));
}

//  Dibujar particulas
void Game::dibujarParticulas()
{
    for (const auto& p : particulas) {
        if (p.vida <= 0.f) continue;
        sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (p.vida / 1.f));
        sf::RectangleShape px(sf::Vector2f(4.f, 4.f));
        px.setOrigin(2.f, 2.f);
        px.setPosition(p.pos);
        px.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, alpha));
        ventana.draw(px);
    }
}
