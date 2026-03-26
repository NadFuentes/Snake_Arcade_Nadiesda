TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  -= qt

QMAKE_CXXFLAGS += -std=c++17

INCLUDEPATH += C:/Aplicaciones/SFML/SFML-2.6.2/include

LIBS += -LC:/Aplicaciones/SFML/SFML-2.6.2/lib \
        -lsfml-graphics \
        -lsfml-window   \
        -lsfml-system   \
        -lsfml-audio

DESTDIR = $$PWD/build

SOURCES += \
    main.cpp     \
    Archivos.cpp \
    UIManager.cpp \
    Game.cpp

HEADERS += \
    Entities.h  \
    Archivos.h  \
    UIManager.h \
    Game.h
