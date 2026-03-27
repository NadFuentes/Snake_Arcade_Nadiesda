#include "UIManager.h"
#include <cstdlib>
#include <cmath>
#include <sstream>


//  Constructor
UIManager::UIManager(sf::RenderWindow& v, sf::Font& f)
    : ventana(v), fuente(f),
    pantallaActual(Pantalla::INICIO),
    nivelSeleccionado(Nivel::NINGUNO),
    modoSeleccionado(ModoJuego::NORMAL),
    tiempo(0.f), timerParpadeo(0.f), cursorVisible(true),
    campoActivo(0), timerError(0.f),
    opcionSel(0)
{
    listaJugadores = GestorArchivos::cargarJugadores();
    initParticulas();
}

//  Particulas de fondo — lluvia de rayitas verdes y azules
void UIManager::initParticulas()
{
    particulas.clear();
    for (int i = 0; i < 90; ++i) {
        Particula p;
        float x = static_cast<float>(std::rand() % static_cast<int>(W() > 0 ? W() : 1280));
        float y = static_cast<float>(std::rand() % static_cast<int>(H() > 0 ? H() : 720));
        float h = static_cast<float>(5 + std::rand() % 18);
        p.shape.setSize(sf::Vector2f(2.f, h));
        p.shape.setPosition(x, y);
        p.velocidad = 35.f + static_cast<float>(std::rand() % 110);
        sf::Uint8 alpha = static_cast<sf::Uint8>(30 + std::rand() % 160);
        if (i % 2 == 0) p.shape.setFillColor(sf::Color(0, 220, 80, alpha));
        else             p.shape.setFillColor(sf::Color(0, 150, 255, alpha));
        particulas.push_back(p);
    }
}

void UIManager::updateParticulas(float dt)
{
    for (auto& p : particulas) {
        p.shape.move(0.f, p.velocidad * dt);
        if (p.shape.getPosition().y > H()) {
            float x = static_cast<float>(std::rand() % static_cast<int>(W()));
            p.shape.setPosition(x, -20.f);
        }
    }
}

void UIManager::drawParticulas()
{
    for (auto& p : particulas) ventana.draw(p.shape);
}

//  Helpers de dibujo
void UIManager::drawFondo()
{
    sf::RectangleShape bg(sf::Vector2f(W(), H()));
    bg.setFillColor(FONDO);
    ventana.draw(bg);
    drawParticulas();
}

void UIManager::drawPanel(float x, float y, float w, float h, sf::Color borde)
{
    // sombra primero
    sf::RectangleShape sh(sf::Vector2f(w, h));
    sh.setPosition(x + 6, y + 6);
    sh.setFillColor(sf::Color(0, 0, 0, 100));
    ventana.draw(sh);

    sf::RectangleShape pn(sf::Vector2f(w, h));
    pn.setPosition(x, y);
    pn.setFillColor(PANEL);
    pn.setOutlineThickness(2.f);
    pn.setOutlineColor(borde);
    ventana.draw(pn);
}

sf::Text UIManager::makeText(const std::string& str, unsigned sz,
                             sf::Color col, float x, float y, bool center)
{
    sf::Text t;
    t.setFont(fuente);
    t.setString(str);
    t.setCharacterSize(sz);
    t.setFillColor(col);
    if (center) {
        auto b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    }
    t.setPosition(x, y);
    return t;
}

void UIManager::drawBoton(const std::string& label, float cx, float cy,
                          float w, float h, bool sel, sf::Color colorSel)
{
    sf::Color fBorde = sel ? colorSel       : sf::Color(0, 90, 50);
    sf::Color fFondo = sel ? sf::Color(0, 50, 28, 220) : sf::Color(5, 20, 12, 180);
    sf::Color fTexto = sel ? AMARILLO        : sf::Color(180, 240, 180);

    sf::RectangleShape btn(sf::Vector2f(w, h));
    btn.setOrigin(w / 2.f, h / 2.f);
    btn.setPosition(cx, cy);
    btn.setFillColor(fFondo);
    btn.setOutlineThickness(sel ? 2.5f : 1.f);
    btn.setOutlineColor(fBorde);
    ventana.draw(btn);

    // glow cuando esta seleccionado
    if (sel) {
        sf::RectangleShape glow(sf::Vector2f(w + 6, h + 6));
        glow.setOrigin((w + 6) / 2.f, (h + 6) / 2.f);
        glow.setPosition(cx, cy);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(1.f);
        glow.setOutlineColor(sf::Color(colorSel.r, colorSel.g, colorSel.b, 80));
        ventana.draw(glow);
    }

    auto txt = makeText(label, 18, fTexto, cx, cy);
    ventana.draw(txt);
}

void UIManager::drawCampo(const std::string& label, const std::string& valor,
                          float cx, float cy, float w, bool activo, bool esPass)
{
    auto lbl = makeText(label, 15, sf::Color(160, 220, 160), cx - w / 2.f, cy - 46.f, false);
    ventana.draw(lbl);

    sf::RectangleShape caja(sf::Vector2f(w, 42.f));
    caja.setOrigin(w / 2.f, 21.f);
    caja.setPosition(cx, cy);
    caja.setFillColor(sf::Color(0, 25, 15, 210));
    caja.setOutlineThickness(activo ? 2.f : 1.f);
    caja.setOutlineColor(activo ? VERDE : sf::Color(0, 80, 45));
    ventana.draw(caja);

    std::string display = esPass ? std::string(valor.size(), '*') : valor;
    display += (activo && cursorVisible) ? "|" : " ";

    auto txt = makeText(display, 17, VERDE, cx, cy);
    ventana.draw(txt);
}

void UIManager::resetCampos()
{
    inputUser  = "";
    inputPass  = "";
    inputPass2 = "";
    campoActivo = 0;
    msgError   = "";
    timerError = 0.f;
    opcionSel  = 0;
}


//  notificarFinPartida — main llama esto cuando termina el Game

void UIManager::notificarFinPartida(int puntaje, ModoJuego modo, int nivelAlcanzado)
{
    // recargar lista actualizada del archivo
    listaJugadores = GestorArchivos::cargarJugadores();

    // refrescar el jugador activo con los datos nuevos del archivo
    Jugador* jPtr = GestorArchivos::validarLogin(
        jugadorActivo.nombre, jugadorActivo.password, listaJugadores);
    if (jPtr) jugadorActivo = *jPtr;
}

//  Loop de actualizacion
void UIManager::actualizar(float dt)
{
    tiempo        += dt;
    timerParpadeo += dt;
    if (timerParpadeo >= 0.53f) { timerParpadeo = 0.f; cursorVisible = !cursorVisible; }
    if (timerError > 0.f) timerError -= dt;
    updateParticulas(dt);
}

void UIManager::procesarEventos(sf::Event& e)
{
    switch (pantallaActual) {
    case Pantalla::INICIO:          eventInicio(e);   break;
    case Pantalla::MENU_PRINCIPAL:  eventMenu(e);     break;
    case Pantalla::LOGIN:           eventLogin(e);    break;
    case Pantalla::CREAR_USUARIO:   eventCrear(e);    break;
    case Pantalla::MENU_JUEGO:      eventMenuJuego(e);break;
    case Pantalla::SELECCION_MODO:  eventSelModo(e);  break;
    case Pantalla::SELECCION_NIVEL: eventSelNivel(e); break;
    case Pantalla::RANKING:         eventRanking(e);  break;
    case Pantalla::CREDITOS:        eventCreditos(e);  break;
    default: break;
    }
}

void UIManager::dibujar()
{
    switch (pantallaActual) {
    case Pantalla::INICIO:          drawInicio();    break;
    case Pantalla::MENU_PRINCIPAL:  drawMenu();      break;
    case Pantalla::LOGIN:           drawLogin();     break;
    case Pantalla::CREAR_USUARIO:   drawCrear();     break;
    case Pantalla::MENU_JUEGO:      drawMenuJuego(); break;
    case Pantalla::SELECCION_MODO:  drawSelModo();   break;
    case Pantalla::SELECCION_NIVEL: drawSelNivel();  break;
    case Pantalla::RANKING:         drawRanking();   break;
    case Pantalla::CREDITOS:        drawCreditos(); break;
    default: break;
    }
}

//  PANTALLA INICIO
void UIManager::drawInicio()
{
    drawFondo();
    float cx = W() / 2.f, cy = H() / 2.f;

    drawPanel(cx - 260, cy - 260, 520, 480, VERDE);

    float pulso = 1.f + 0.04f * std::sin(tiempo * 2.5f);
    sf::Uint8 ga = static_cast<sf::Uint8>(200 + 55 * std::sin(tiempo * 2.f));
    auto titulo = makeText("BIENVENIDO", 38, sf::Color(0, ga, 80), cx, cy - 190);
    titulo.setScale(pulso, pulso);
    ventana.draw(titulo);

    auto sub = makeText("Podras lograrlo?", 20, AZUL, cx, cy - 135);
    ventana.draw(sub);

    sf::RectangleShape sep(sf::Vector2f(380, 2));
    sep.setOrigin(190, 1); sep.setPosition(cx, cy - 100);
    sep.setFillColor(sf::Color(0, 180, 255, 160));
    ventana.draw(sep);

    auto nombre = makeText("SNAKE  ARCADE", 28, VERDE, cx, cy - 50);
    ventana.draw(nombre);

    drawBoton("  >  PLAY  <  ", cx, cy + 60, 260, 58, true, VERDE);

    sf::Uint8 pa = static_cast<sf::Uint8>(120 + 100 * std::sin(tiempo * 3.f));
    auto inst = makeText("Presiona ENTER o haz clic en PLAY", 13,
                         sf::Color(100, 180, 100, pa), cx, cy + 140);
    ventana.draw(inst);
}

void UIManager::eventInicio(sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed &&
        (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::Space)) {
        resetCampos();
        pantallaActual = Pantalla::MENU_PRINCIPAL;
    }
    if (e.type == sf::Event::MouseButtonReleased &&
        e.mouseButton.button == sf::Mouse::Left) {
        float cx = W()/2.f, cy = H()/2.f;
        float mx = static_cast<float>(e.mouseButton.x);
        float my = static_cast<float>(e.mouseButton.y);
        if (mx >= cx-130 && mx <= cx+130 && my >= cy+31 && my <= cy+89) {
            resetCampos();
            pantallaActual = Pantalla::MENU_PRINCIPAL;
        }
    }
}

//  MENU PRINCIPAL
void UIManager::drawMenu()
{
    drawFondo();
    float cx = W()/2.f, cy = H()/2.f;

    drawPanel(cx - 240, cy - 230, 480, 420, AZUL);

    auto titulo = makeText("MENU PRINCIPAL", 26, AZUL, cx, cy - 190);
    ventana.draw(titulo);

    sf::RectangleShape sep(sf::Vector2f(380, 2));
    sep.setOrigin(190,1); sep.setPosition(cx, cy-160);
    sep.setFillColor(sf::Color(0,220,80,150));
    ventana.draw(sep);

    const char* labels[] = {"INICIAR SESION", "CREAR USUARIO", "REGRESAR"};
    sf::Color colores[]  = {VERDE, AZUL, sf::Color(200,200,200)};
    float yBase = cy - 80.f;
    for (int i = 0; i < 3; ++i)
        drawBoton(labels[i], cx, yBase + i*100.f, 300, 54, (opcionSel==i), colores[i]);

    auto inst = makeText("[ FLECHAS ] Mover   [ ENTER ] Seleccionar", 12,
                         sf::Color(80,150,80,180), cx, cy + 205);
    ventana.draw(inst);
}

void UIManager::eventMenu(sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Up)   opcionSel = (opcionSel-1+3)%3;
        if (e.key.code == sf::Keyboard::Down)  opcionSel = (opcionSel+1)%3;
        if (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::Space) {
            resetCampos();
            if (opcionSel==0) pantallaActual = Pantalla::LOGIN;
            if (opcionSel==1) pantallaActual = Pantalla::CREAR_USUARIO;
            if (opcionSel==2) pantallaActual = Pantalla::INICIO;
        }
    }
    if (e.type == sf::Event::MouseButtonReleased &&
        e.mouseButton.button == sf::Mouse::Left) {
        float cx = W()/2.f, cy = H()/2.f;
        float mx = static_cast<float>(e.mouseButton.x);
        float my = static_cast<float>(e.mouseButton.y);
        float yBase = cy - 80.f;
        for (int i = 0; i < 3; ++i) {
            float bx=cx-150, by=yBase+i*100.f-27, bw=300, bh=54;
            if (mx>=bx&&mx<=bx+bw&&my>=by&&my<=by+bh) {
                opcionSel=i; resetCampos();
                if (i==0) pantallaActual=Pantalla::LOGIN;
                if (i==1) pantallaActual=Pantalla::CREAR_USUARIO;
                if (i==2) pantallaActual=Pantalla::INICIO;
            }
        }
    }
}


//  LOGIN
void UIManager::drawLogin()
{
    drawFondo();
    float cx=W()/2.f, cy=H()/2.f;

    drawPanel(cx-250, cy-270, 500, 490, AZUL);
    ventana.draw(makeText("INICIAR SESION", 26, AZUL, cx, cy-228));

    sf::RectangleShape sep(sf::Vector2f(400,2));
    sep.setOrigin(200,1); sep.setPosition(cx, cy-200);
    sep.setFillColor(VERDE);
    ventana.draw(sep);

    drawCampo("USERNAME:", inputUser, cx, cy-120, 380, campoActivo==0, false);
    drawCampo("PASSWORD:", inputPass, cx, cy-20,  380, campoActivo==1, true);

    ventana.draw(makeText("(max. 8 caracteres)", 12,
                          sf::Color(100,160,100,180), cx, cy+20));

    if (timerError > 0.f)
        ventana.draw(makeText(msgError, 14, ROJO, cx, cy+70));

    drawBoton("ENTRAR",   cx-90, cy+140, 150, 48, opcionSel==0, VERDE);
    drawBoton("CANCELAR", cx+90, cy+140, 150, 48, opcionSel==1, sf::Color(200,200,200));

    ventana.draw(makeText("[ TAB ] Cambiar campo   [ ESC ] Volver", 12,
                          sf::Color(80,150,80,180), cx, cy+210));
}

void UIManager::eventLogin(sf::Event& e)
{
    if (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Tab)
        campoActivo=(campoActivo+1)%2;

    if (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape)
    { resetCampos(); pantallaActual=Pantalla::MENU_PRINCIPAL; }

    if (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Return) {
        if (opcionSel==1) { resetCampos(); pantallaActual=Pantalla::MENU_PRINCIPAL; return; }
        if (inputUser.empty()||inputPass.empty())
        { msgError="Completa todos los campos"; timerError=2.5f; return; }
        Jugador* j=GestorArchivos::validarLogin(inputUser,inputPass,listaJugadores);
        if (j) {
            jugadorActivo=*j; resetCampos();
            // al iniciar sesion muestra el ranking primero — estilo arcade
            pantallaActual=Pantalla::RANKING;
        } else {
            msgError="Usuario o contrasena incorrectos"; timerError=2.5f; inputPass="";
        }
    }

    if (e.type==sf::Event::KeyPressed) {
        if (e.key.code==sf::Keyboard::Left)  opcionSel=0;
        if (e.key.code==sf::Keyboard::Right) opcionSel=1;
    }

    if (e.type==sf::Event::TextEntered) {
        auto& campo=(campoActivo==0)?inputUser:inputPass;
        int maxLen=(campoActivo==1)?8:32;
        if (e.text.unicode==8) { if(!campo.empty()) campo.pop_back(); }
        else if (e.text.unicode>=32&&e.text.unicode<128)
            if ((int)campo.size()<maxLen) campo+=(char)e.text.unicode;
    }

    if (e.type==sf::Event::MouseButtonReleased&&e.mouseButton.button==sf::Mouse::Left) {
        float cx=W()/2.f, cy=H()/2.f;
        float mx=static_cast<float>(e.mouseButton.x);
        float my=static_cast<float>(e.mouseButton.y);
        if (mx>=cx-190&&mx<=cx+190&&my>=cy-141&&my<=cy-99) campoActivo=0;
        if (mx>=cx-190&&mx<=cx+190&&my>=cy- 41&&my<=cy+ 1) campoActivo=1;
        if (mx>=cx-165&&mx<=cx-15&&my>=cy+116&&my<=cy+164) {
            opcionSel=0;
            if (inputUser.empty()||inputPass.empty()){msgError="Completa todos los campos";timerError=2.5f;return;}
            Jugador* j=GestorArchivos::validarLogin(inputUser,inputPass,listaJugadores);
            if (j) { jugadorActivo=*j; resetCampos(); pantallaActual=Pantalla::RANKING; }
            else   { msgError="Usuario o contrasena incorrectos"; timerError=2.5f; inputPass=""; }
        }
        if (mx>=cx+15&&mx<=cx+165&&my>=cy+116&&my<=cy+164)
        { resetCampos(); pantallaActual=Pantalla::MENU_PRINCIPAL; }
    }
}

//  CREAR USUARIO
void UIManager::drawCrear()
{
    drawFondo();
    float cx=W()/2.f, cy=H()/2.f;

    drawPanel(cx-260, cy-300, 520, 550, VERDE);
    ventana.draw(makeText("CREAR USUARIO", 26, VERDE, cx, cy-258));

    sf::RectangleShape sep(sf::Vector2f(420,2));
    sep.setOrigin(210,1); sep.setPosition(cx, cy-228);
    sep.setFillColor(AZUL);
    ventana.draw(sep);

    drawCampo("USERNAME:",             inputUser,  cx, cy-148, 400, campoActivo==0, false);
    drawCampo("CONTRASENA:",           inputPass,  cx, cy- 48, 400, campoActivo==1, true);
    drawCampo("CONFIRMAR CONTRASENA:", inputPass2, cx, cy+ 62, 400, campoActivo==2, true);

    ventana.draw(makeText("(max. 8 caracteres para contrasena)", 12,
                          sf::Color(100,160,100,180), cx, cy+100));

    if (timerError > 0.f)
        ventana.draw(makeText(msgError, 14, ROJO, cx, cy+130));

    drawBoton("CREAR Y ENTRAR", cx-90, cy+190, 210, 50, opcionSel==0, VERDE);
    drawBoton("CANCELAR",       cx+90, cy+190, 150, 50, opcionSel==1, sf::Color(200,200,200));

    ventana.draw(makeText("[ TAB ] Cambiar campo   [ ESC ] Volver", 12,
                          sf::Color(80,150,80,180), cx, cy+255));
}

void UIManager::eventCrear(sf::Event& e)
{
    if (e.type==sf::Event::KeyPressed&&e.key.code==sf::Keyboard::Tab)
        campoActivo=(campoActivo+1)%3;

    if (e.type==sf::Event::KeyPressed&&e.key.code==sf::Keyboard::Escape)
    { resetCampos(); pantallaActual=Pantalla::MENU_PRINCIPAL; }

    if (e.type==sf::Event::KeyPressed) {
        if (e.key.code==sf::Keyboard::Left)  opcionSel=0;
        if (e.key.code==sf::Keyboard::Right) opcionSel=1;
    }

    auto intentarCrear = [&]() {
        if (inputUser.empty()||inputPass.empty()||inputPass2.empty())
        { msgError="Completa todos los campos"; timerError=2.5f; return; }
        if (inputUser.size()<3)
        { msgError="Username: minimo 3 caracteres"; timerError=2.5f; return; }
        if (inputPass!=inputPass2)
        { msgError="Las contrasenas no coinciden"; timerError=2.5f; inputPass2=""; return; }
        if (GestorArchivos::existeJugador(inputUser,listaJugadores))
        { msgError="Ese username ya existe"; timerError=2.5f; inputUser=""; return; }
        Jugador nuevo(inputUser, inputPass);
        GestorArchivos::agregarJugador(nuevo, listaJugadores);
        jugadorActivo=nuevo;
        resetCampos();
        pantallaActual=Pantalla::RANKING; // al crear tambin muestra el ranking
    };

    if (e.type==sf::Event::KeyPressed&&e.key.code==sf::Keyboard::Return) {
        if (opcionSel==1){resetCampos();pantallaActual=Pantalla::MENU_PRINCIPAL;return;}
        intentarCrear();
    }

    if (e.type==sf::Event::TextEntered) {
        std::string* campo=nullptr; int maxLen=32;
        if (campoActivo==0) { campo=&inputUser; maxLen=32; }
        if (campoActivo==1) { campo=&inputPass; maxLen=8;  }
        if (campoActivo==2) { campo=&inputPass2;maxLen=8;  }
        if (!campo) return;
        if (e.text.unicode==8) { if(!campo->empty()) campo->pop_back(); }
        else if(e.text.unicode>=32&&e.text.unicode<128)
            if((int)campo->size()<maxLen) *campo+=(char)e.text.unicode;
    }

    if (e.type==sf::Event::MouseButtonReleased&&e.mouseButton.button==sf::Mouse::Left) {
        float cx=W()/2.f, cy=H()/2.f;
        float mx=static_cast<float>(e.mouseButton.x);
        float my=static_cast<float>(e.mouseButton.y);
        if(mx>=cx-200&&mx<=cx+200&&my>=cy-169&&my<=cy-127) campoActivo=0;
        if(mx>=cx-200&&mx<=cx+200&&my>=cy- 69&&my<=cy- 27) campoActivo=1;
        if(mx>=cx-200&&mx<=cx+200&&my>=cy+ 41&&my<=cy+ 83) campoActivo=2;
        if(mx>=cx-195&&mx<=cx+15&&my>=cy+165&&my<=cy+215){ opcionSel=0; intentarCrear(); }
        if(mx>=cx+15&&mx<=cx+165&&my>=cy+165&&my<=cy+215){ resetCampos();pantallaActual=Pantalla::MENU_PRINCIPAL; }
    }
}


//  MENU JUEGO — despues de iniciar sesion
void UIManager::drawMenuJuego()
{
    drawFondo();
    float cx = W()/2.f, cy = H()/2.f;

    // panel mas alto para acomodar 5 botones
    drawPanel(cx-270, cy-300, 540, 560, VERDE);

    auto titulo = makeText("MENU JUEGO", 28, VERDE, cx, cy-260);
    ventana.draw(titulo);

    if (!jugadorActivo.nombre.empty()) {
        auto bv = makeText("Jugador: " + jugadorActivo.nombre, 15, AMARILLO, cx, cy-218);
        ventana.draw(bv);
    }

    sf::RectangleShape sep(sf::Vector2f(440, 2));
    sep.setOrigin(220, 1); sep.setPosition(cx, cy-192);
    sep.setFillColor(sf::Color(0, 180, 255, 160));
    ventana.draw(sep);

    // 5 botones: Jugar, Ranking, Creditos, Cerrar Sesion, Regresar
    float yBase = cy - 140.f;
    float paso  = 78.f;

    drawBoton("JUGAR",          cx, yBase + 0*paso, 300, 58, opcionSel==0, VERDE);
    drawBoton("VER RANKING",    cx, yBase + 1*paso, 300, 58, opcionSel==1, AZUL);
    drawBoton("CREDITOS",       cx, yBase + 2*paso, 300, 58, opcionSel==2, MORADO);
    drawBoton("CERRAR SESION",  cx, yBase + 3*paso, 300, 58, opcionSel==3, sf::Color(255,120,0));
    drawBoton("REGRESAR",       cx, yBase + 4*paso, 300, 58, opcionSel==4, sf::Color(200,200,200));

    // mejor puntaje del jugador
    std::ostringstream info;
    info << "Normal: " << jugadorActivo.puntajeNormal
         << "  |  Competitivo: " << jugadorActivo.puntajeCompetitivo;
    ventana.draw(makeText(info.str(), 13, sf::Color(120,200,120), cx, cy+228));

    ventana.draw(makeText("[ FLECHAS ] Mover   [ ENTER ] Seleccionar", 12,
                          sf::Color(80,150,80,180), cx, cy+252));
}

void UIManager::eventMenuJuego(sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Up)   opcionSel = (opcionSel-1+5)%5;
        if (e.key.code == sf::Keyboard::Down) opcionSel = (opcionSel+1)%5;

        if (e.key.code == sf::Keyboard::Return || e.key.code == sf::Keyboard::Space) {
            if (opcionSel == 0) { resetCampos(); pantallaActual = Pantalla::SELECCION_MODO; }
            if (opcionSel == 1) { opcionSel = 0; pantallaActual = Pantalla::RANKING; }
            if (opcionSel == 2) { opcionSel = 0; pantallaActual = Pantalla::CREDITOS; }
            if (opcionSel == 3) {
                // cerrar sesion — limpia el jugador activo y va al menu principal
                // NO cierra la ventana, solo desloguea
                jugadorActivo = Jugador();
                resetCampos();
                pantallaActual = Pantalla::MENU_PRINCIPAL;
            }
            if (opcionSel == 4) {
                // regresar va al inicio (ya estaba)
                jugadorActivo = Jugador();
                resetCampos();
                pantallaActual = Pantalla::INICIO;
            }
        }
    }

    if (e.type == sf::Event::MouseButtonReleased &&
        e.mouseButton.button == sf::Mouse::Left) {
        float cx = W()/2.f, cy = H()/2.f;
        float mx = static_cast<float>(e.mouseButton.x);
        float my = static_cast<float>(e.mouseButton.y);
        float yBase = cy - 140.f;
        float paso  = 78.f;

        for (int i = 0; i < 5; ++i) {
            float bx = cx-150, by = yBase + i*paso - 29, bw = 300, bh = 58;
            if (mx >= bx && mx <= bx+bw && my >= by && my <= by+bh) {
                opcionSel = i;
                if (i==0) { resetCampos(); pantallaActual = Pantalla::SELECCION_MODO; }
                if (i==1) { opcionSel=0;   pantallaActual = Pantalla::RANKING; }
                if (i==2) { opcionSel=0;   pantallaActual = Pantalla::CREDITOS; }
                if (i==3) { jugadorActivo = Jugador(); resetCampos(); pantallaActual = Pantalla::MENU_PRINCIPAL; }
                if (i==4) { jugadorActivo = Jugador(); resetCampos(); pantallaActual = Pantalla::INICIO; }
            }
        }
    }
}

void UIManager::drawCreditos()
{
    drawFondo();
    float cx = W()/2.f, cy = H()/2.f;

    // panel con borde morado
    drawPanel(cx-300, cy-280, 600, 520, MORADO);

    // titulo animado
    float pulso = 1.f + 0.03f * std::sin(tiempo * 2.8f);
    sf::Uint8 ma = static_cast<sf::Uint8>(200 + 55 * std::sin(tiempo * 2.f));
    auto titulo = makeText("CREDITOS", 36, sf::Color(180, 80, 255, ma), cx, cy-240);
    titulo.setScale(pulso, pulso);
    ventana.draw(titulo);

    // --- Imagen arriba ---
    static sf::Texture fotoTex;
    static bool cargada = false;
    if (!cargada) {
        if (fotoTex.loadFromFile("assets/foto3.png")) {
            cargada = true;
        }
    }
    if (cargada) {
        sf::Sprite foto(fotoTex);
        foto.setOrigin(fotoTex.getSize().x/2.f, fotoTex.getSize().y/2.f);
        foto.setPosition(cx, cy-120); // debajo del título
        foto.setScale(0.15f, 0.15f);  // tamaño reducido
        ventana.draw(foto);
    }

    // --- Información personal debajo de la imagen ---
    ventana.draw(makeText("Programador", 20, sf::Color(128, 128, 128), cx, cy+20));
    ventana.draw(makeText("Nadiesda Fuentes", 22, sf::Color(255, 215, 0), cx, cy+50));
    ventana.draw(makeText("nadfuentes.hdez03@gmail.com", 18, sf::Color(100, 255, 160), cx, cy+80));

    // --- Agradecimiento ---
    ventana.draw(makeText("Agradecimiento especial al Ing. Idiaquez G.", 18,
                          sf::Color(200, 200, 200), cx, cy+120));

    // separador final
    sf::RectangleShape sep3(sf::Vector2f(480, 1));
    sep3.setOrigin(240, 0); sep3.setPosition(cx, cy+160);
    sep3.setFillColor(sf::Color(100, 100, 100, 180));
    ventana.draw(sep3);

    // instruccion para volver
    sf::Uint8 ia = static_cast<sf::Uint8>(100 + 100 * std::sin(tiempo * 3.f));
    ventana.draw(makeText("[ ESC / ENTER ] Volver al menu", 13,
                          sf::Color(180, 240, 180, ia), cx, cy+190));

    // --- BOTÓN REGRESAR (dentro del panel) ---
    drawBoton("REGRESAR", cx, cy+220, 220, 50, false, sf::Color(200,200,200));
}





void UIManager::eventCreditos(sf::Event& e)
{
    // cualquier tecla de confirmacion vuelve al menu del juego
    if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Escape ||
            e.key.code == sf::Keyboard::Return ||
            e.key.code == sf::Keyboard::Space) {
            opcionSel = 0;
            pantallaActual = Pantalla::MENU_JUEGO;
        }
    }

    // clic en cualquier parte tambien vuelve
    if (e.type == sf::Event::MouseButtonReleased &&
        e.mouseButton.button == sf::Mouse::Left) {
        opcionSel = 0;
        pantallaActual = Pantalla::MENU_JUEGO;
    }
}


//  SELECCION DE MODO — normal o competitivo
void UIManager::drawSelModo()
{
    drawFondo();
    float cx=W()/2.f, cy=H()/2.f;

    drawPanel(cx-310, cy-280, 620, 520, AMARILLO);
    ventana.draw(makeText("SELECCIONA MODO", 26, AMARILLO, cx, cy-244));

    if (!jugadorActivo.nombre.empty())
        ventana.draw(makeText("Jugador: " + jugadorActivo.nombre, 14, VERDE, cx, cy-205));

    sf::RectangleShape sep(sf::Vector2f(500,2));
    sep.setOrigin(250,1); sep.setPosition(cx, cy-182);
    sep.setFillColor(AZUL);
    ventana.draw(sep);

    // ---- Modo Normal ----
    bool selNormal = (opcionSel==0);
    sf::RectangleShape cajaN(sf::Vector2f(520, 110));
    cajaN.setOrigin(260,55); cajaN.setPosition(cx, cy-90);
    cajaN.setFillColor(selNormal ? sf::Color(0,40,22,230) : sf::Color(5,18,12,180));
    cajaN.setOutlineThickness(selNormal ? 2.5f : 1.f);
    cajaN.setOutlineColor(selNormal ? VERDE : sf::Color(0,80,40));
    ventana.draw(cajaN);

    ventana.draw(makeText("NORMAL", 20,
                          selNormal ? VERDE : sf::Color(180,240,180), cx, cy-115));
    ventana.draw(makeText("Elige cualquier nivel libremente", 14,
                          sf::Color(150,215,150), cx, cy-85));
    ventana.draw(makeText("Puntaje independiente por nivel", 13,
                          sf::Color(120,190,120), cx, cy-65));

    // ---- Modo Competitivo ----
    bool selComp = (opcionSel==1);
    sf::RectangleShape cajaC(sf::Vector2f(520, 130));
    cajaC.setOrigin(260,65); cajaC.setPosition(cx, cy+70);
    cajaC.setFillColor(selComp ? sf::Color(40,30,0,230) : sf::Color(5,18,12,180));
    cajaC.setOutlineThickness(selComp ? 2.5f : 1.f);
    cajaC.setOutlineColor(selComp ? AMARILLO : sf::Color(0,80,40));
    ventana.draw(cajaC);

    ventana.draw(makeText("COMPETITIVO", 20,
                          selComp ? AMARILLO : sf::Color(220,200,120), cx, cy+35));
    ventana.draw(makeText("Empieza en el nivel 1, sube al comer 25 cerezas", 14,
                          sf::Color(200,180,100), cx, cy+65));
    ventana.draw(makeText("Nivel 3: aparecen cerezas doradas (x2 puntos)!", 13,
                          sf::Color(180,160,80), cx, cy+85));

    // mostrar nivel desbloqueado en competitivo
    std::string nivelDesc = "Tu nivel desbloqueado: " +
                            std::to_string(jugadorActivo.nivelDesbloqueado);
    ventana.draw(makeText(nivelDesc, 12,
                          sf::Color(255,215,0,180), cx, cy+110));

    ventana.draw(makeText("[ FLECHAS ] Mover   [ ENTER ] Seleccionar   [ ESC ] Volver", 12,
                          sf::Color(80,150,80,180), cx, cy+200));
}

void UIManager::eventSelModo(sf::Event& e)
{
    if (e.type==sf::Event::KeyPressed) {
        if (e.key.code==sf::Keyboard::Up||e.key.code==sf::Keyboard::Down)
            opcionSel=(opcionSel==0)?1:0;

        if (e.key.code==sf::Keyboard::Escape)
        { opcionSel=0; pantallaActual=Pantalla::MENU_JUEGO; }

        if (e.key.code==sf::Keyboard::Return||e.key.code==sf::Keyboard::Space) {
            if (opcionSel==0) {
                // modo normal — va a elegir nivel
                modoSeleccionado = ModoJuego::NORMAL;
                opcionSel=0;
                pantallaActual=Pantalla::SELECCION_NIVEL;
            } else {
                // modo competitivo — empieza siempre en nivel 1
                modoSeleccionado  = ModoJuego::COMPETITIVO;
                nivelSeleccionado = Nivel::UNO;
                pantallaActual    = Pantalla::JUEGO;
            }
        }
    }
    if (e.type==sf::Event::MouseButtonReleased&&e.mouseButton.button==sf::Mouse::Left) {
        float cx=W()/2.f, cy=H()/2.f;
        float mx=static_cast<float>(e.mouseButton.x);
        float my=static_cast<float>(e.mouseButton.y);
        // clic en normal
        if (mx>=cx-260&&mx<=cx+260&&my>=cy-145&&my<=cy-35)
        { opcionSel=0; modoSeleccionado=ModoJuego::NORMAL; pantallaActual=Pantalla::SELECCION_NIVEL; }
        // clic en competitivo
        if (mx>=cx-260&&mx<=cx+260&&my>=cy+5&&my<=cy+135)
        { opcionSel=1; modoSeleccionado=ModoJuego::COMPETITIVO; nivelSeleccionado=Nivel::UNO; pantallaActual=Pantalla::JUEGO; }
    }
}

//  Seleccion de nivel para el modo de uego normal
void UIManager::drawSelNivel()
{
    drawFondo();
    float cx=W()/2.f, cy=H()/2.f;

    drawPanel(cx-290, cy-280, 580, 530, VERDE);
    ventana.draw(makeText("SELECCIONA NIVEL", 24, VERDE, cx, cy-244));

    if (!jugadorActivo.nombre.empty())
        ventana.draw(makeText("Jugador: " + jugadorActivo.nombre, 14, AMARILLO, cx, cy-208));

    sf::RectangleShape sep(sf::Vector2f(480,2));
    sep.setOrigin(240,1); sep.setPosition(cx, cy-184);
    sep.setFillColor(AZUL);
    ventana.draw(sep);

    struct InfoNivel { const char* nombre; const char* d1; const char* d2; sf::Color color; };
    InfoNivel niveles[3] = {
        {"NIVEL 1 - CLASICO",  "Velocidad constante",   "Sin obstaculos",                 sf::Color(0,220,80)},
        {"NIVEL 2 - AVANZADO", "Velocidad incremental", "Muros perimetrales activos",      sf::Color(0,160,255)},
        {"NIVEL 3 - EXTREMO",  "Obstaculos aleatorios", "Frutas doradas x2 puntos!",       sf::Color(255,180,0)}
    };

    float yBase=cy-120.f;
    for (int i=0; i<3; ++i) {
        bool sel=(opcionSel==i);
        float yn=yBase+i*110.f;

        sf::RectangleShape caja(sf::Vector2f(480,90));
        caja.setOrigin(240,45); caja.setPosition(cx,yn);
        caja.setFillColor(sel ? sf::Color(0,40,22,230) : sf::Color(5,18,12,180));
        caja.setOutlineThickness(sel?2.f:1.f);
        caja.setOutlineColor(sel ? niveles[i].color : sf::Color(0,80,40));
        ventana.draw(caja);

        auto nom = makeText(niveles[i].nombre, 15,
                            sel ? niveles[i].color : sf::Color(180,240,180), cx, yn-22);
        ventana.draw(nom);
        ventana.draw(makeText(std::string("+ ") + niveles[i].d1, 13,
                              sf::Color(150,215,150), cx, yn+2));
        ventana.draw(makeText(std::string("+ ") + niveles[i].d2, 13,
                              sf::Color(130,195,130), cx, yn+22));
    }

    ventana.draw(makeText("[ ENTER ] Jugar   [ ESC ] Volver", 12,
                          sf::Color(80,150,80,180), cx, cy+225));
}

void UIManager::eventSelNivel(sf::Event& e)
{
    if (e.type==sf::Event::KeyPressed) {
        if (e.key.code==sf::Keyboard::Up)   opcionSel=(opcionSel-1+3)%3;
        if (e.key.code==sf::Keyboard::Down) opcionSel=(opcionSel+1)%3;
        if (e.key.code==sf::Keyboard::Escape){ opcionSel=0; pantallaActual=Pantalla::SELECCION_MODO; }
        if (e.key.code==sf::Keyboard::Return||e.key.code==sf::Keyboard::Space) {
            nivelSeleccionado=static_cast<Nivel>(opcionSel+1);
            pantallaActual=Pantalla::JUEGO;
        }
    }
    if (e.type==sf::Event::MouseButtonReleased&&e.mouseButton.button==sf::Mouse::Left) {
        float cx=W()/2.f, cy=H()/2.f;
        float mx=static_cast<float>(e.mouseButton.x);
        float my=static_cast<float>(e.mouseButton.y);
        float yBase=cy-120.f;
        for (int i=0;i<3;++i) {
            float yn=yBase+i*110.f;
            if (mx>=cx-240&&mx<=cx+240&&my>=yn-45&&my<=yn+45) {
                opcionSel=i;
                nivelSeleccionado=static_cast<Nivel>(i+1);
                pantallaActual=Pantalla::JUEGO;
            }
        }
    }
}


//Tabla de ranking
void UIManager::drawRanking()
{
    drawFondo();
    float cx=W()/2.f, cy=H()/2.f;

    // panel grande
    drawPanel(cx-340, cy-310, 680, 580, AMARILLO);

    // titulo parpadeante
    sf::Uint8 ta = static_cast<sf::Uint8>(200 + 55*std::sin(tiempo * 3.f));
    ventana.draw(makeText("HIGH SCORES", 32, sf::Color(255,215,0,ta), cx, cy-272));

    // tabs para cambiar entre modos
    float tabY = cy - 235.f;
    drawBoton("MODO NORMAL",      cx-130, tabY, 220, 38, opcionSel==0, VERDE);
    drawBoton("MODO COMPETITIVO", cx+140, tabY, 240, 38, opcionSel==1, AMARILLO);

    sf::RectangleShape sep(sf::Vector2f(600,2));
    sep.setOrigin(300,1); sep.setPosition(cx, cy-210);
    sep.setFillColor(sf::Color(100,100,100,150));
    ventana.draw(sep);

    // cargar y ordenar la lista segun el tab seleccionado
    ModoJuego modoRank = (opcionSel==0) ? ModoJuego::NORMAL : ModoJuego::COMPETITIVO;
    std::vector<Jugador> lista = listaJugadores;
    ordenarPorPuntaje(lista, modoRank);

    // encabezados de columna
    float colRank  = cx - 260.f;
    float colNom   = cx - 160.f;
    float colPunt  = cx + 100.f;
    float colNivMax = cx + 240.f;

    ventana.draw(makeText("#",       14, sf::Color(150,150,150), colRank,  cy-185, false));
    ventana.draw(makeText("JUGADOR", 14, sf::Color(150,150,150), colNom,   cy-185, false));
    ventana.draw(makeText("PUNTAJE", 14, sf::Color(150,150,150), colPunt,  cy-185, false));
    if (opcionSel==1)
        ventana.draw(makeText("NIVEL MAX", 14, sf::Color(150,150,150), colNivMax, cy-185, false));

    // filas — maximo 8 entradas
    int maxEntradas = 8;
    float rowH = 44.f;
    float yStart = cy - 155.f;

    for (int i=0; i<static_cast<int>(lista.size()) && i<maxEntradas; ++i) {
        const Jugador& j = lista[i];
        int punt = (modoRank==ModoJuego::NORMAL) ? j.puntajeNormal : j.puntajeCompetitivo;

        // saltamos jugadores con 0 puntaje — solo mostrar quien ha jugado
        if (punt == 0) continue;

        float fy = yStart + i * rowH;

        // resaltar la fila del jugador actual
        bool esMio = (j.nombre == jugadorActivo.nombre);
        sf::Color colFila = esMio ? sf::Color(0,60,30,160) : sf::Color(0,0,0,0);
        if (esMio) {
            sf::RectangleShape hilite(sf::Vector2f(620, rowH-4));
            hilite.setPosition(cx-310, fy-8);
            hilite.setFillColor(colFila);
            hilite.setOutlineThickness(1.f);
            hilite.setOutlineColor(VERDE);
            ventana.draw(hilite);
        }

        // colores especiales para top 3
        sf::Color cRank;
        if      (i==0) cRank = sf::Color(255,215,0);   // oro
        else if (i==1) cRank = sf::Color(192,192,192); // plata
        else if (i==2) cRank = sf::Color(205,127,50);  // bronce
        else           cRank = sf::Color(180,220,180);

        std::string rankStr = "#" + std::to_string(i+1);
        if (i==0) rankStr = "TOP 1";

        ventana.draw(makeText(rankStr,      16, cRank,          colRank,  fy, false));
        ventana.draw(makeText(j.nombre,     16, esMio?VERDE:sf::Color(220,240,220), colNom, fy, false));
        ventana.draw(makeText(std::to_string(punt), 16, AMARILLO, colPunt, fy, false));

        if (opcionSel==1)
            ventana.draw(makeText("Nv." + std::to_string(j.nivelMaxCompetitivo),
                                  16, AZUL, colNivMax, fy, false));
    }

    // si no hay entradas todavia
    bool hayEntradas = false;
    for (const auto& j : lista) {
        int p = (modoRank==ModoJuego::NORMAL) ? j.puntajeNormal : j.puntajeCompetitivo;
        if (p > 0) { hayEntradas = true; break; }
    }
    if (!hayEntradas) {
        ventana.draw(makeText("Nadie ha jugado aun en este modo...", 18,
                              sf::Color(120,120,120), cx, cy+10));
    }

    // nota de quien esta logueado
    ventana.draw(makeText("Tu jugador: " + jugadorActivo.nombre, 13,
                          VERDE, cx, cy+215));

    // instrucciones para salir
    ventana.draw(makeText("[ TAB ] Cambiar modo   [ ENTER / ESC ] Continuar", 12,
                          sf::Color(80,150,80,180), cx, cy+250));
}

void UIManager::eventRanking(sf::Event& e)
{
    if (e.type==sf::Event::KeyPressed) {
        // tab cambia entre ranking normal y competitivo
        if (e.key.code==sf::Keyboard::Tab)
            opcionSel=(opcionSel==0)?1:0;

        // enter o esc van al menu del juego
        if (e.key.code==sf::Keyboard::Return || e.key.code==sf::Keyboard::Escape)
            pantallaActual=Pantalla::MENU_JUEGO;
    }
    // clic en los tabs
    if (e.type==sf::Event::MouseButtonReleased&&e.mouseButton.button==sf::Mouse::Left) {
        float cx=W()/2.f, cy=H()/2.f;
        float mx=static_cast<float>(e.mouseButton.x);
        float my=static_cast<float>(e.mouseButton.y);
        float tabY=cy-235.f;
        if (mx>=cx-240&&mx<=cx-20&&my>=tabY-19&&my<=tabY+19)  opcionSel=0;
        if (mx>=cx+20&&mx<=cx+260&&my>=tabY-19&&my<=tabY+19)  opcionSel=1;
    }
}
