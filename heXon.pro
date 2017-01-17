TARGET = hexon

LIBS += ../heXon/Urho3D/lib/libUrho3D.a \
    -lpthread \
    -ldl \
    -lGL

QMAKE_CXXFLAGS += -std=c++14

INCLUDEPATH += \
    ../heXon/Urho3D/include \
    ../heXon/Urho3D/include/Urho3D/ThirdParty \

TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    apple.cpp \
    bubble.cpp \
    bullet.cpp \
    chaoball.cpp \
    chaoflash.cpp \
    chaomine.cpp \
    chaozap.cpp \
    controllable.cpp \
    door.cpp \
    effect.cpp \
    effectmaster.cpp \
    enemy.cpp \
    explosion.cpp \
    flash.cpp \
    heart.cpp \
    hexocam.cpp \
    hitfx.cpp \
    inputmaster.cpp \
    line.cpp \
    luckey.cpp \
    mastercontrol.cpp \
    muzzle.cpp \
    pickup.cpp \
    player.cpp \
    razor.cpp \
    sceneobject.cpp \
    seeker.cpp \
    spawnmaster.cpp \
    spire.cpp \
    splatterpillar.cpp \
    TailGenerator.cpp \
    tile.cpp \
    pilot.cpp \
    phaser.cpp \
    arena.cpp \
    lobby.cpp \
    highest.cpp \
    ship.cpp \
    gui3d.cpp \
    panel.cpp

HEADERS += \
    apple.h \
    bubble.h \
    bullet.h \
    chaoball.h \
    chaoflash.h \
    chaomine.h \
    chaozap.h \
    controllable.h \
    door.h \
    effect.h \
    effectmaster.h \
    enemy.h \
    explosion.h \
    flash.h \
    heart.h \
    hexocam.h \
    hitfx.h \
    inputmaster.h \
    line.h \
    luckey.h \
    mastercontrol.h \
    muzzle.h \
    pickup.h \
    player.h \
    razor.h \
    sceneobject.h \
    seeker.h \
    spawnmaster.h \
    spire.h \
    splatterpillar.h \
    TailGenerator.h \
    tile.h \
    pilot.h \
    phaser.h \
    arena.h \
    lobby.h \
    highest.h \
    ship.h \
    hexonevents.h \
    gui3d.h \
    panel.h

OTHER_FILES += \
    Docs/Todo.md

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = $$PREFIX/bin
    }
    isEmpty(DATADIR) {
        DATADIR = ~/.local/share
    }
    DEFINES += DATADIR=\\\"$${DATADIR}/hexon\\\"

    target.path = $$BINDIR

    icon.files = hexon.svg
    icon.path = $$DATADIR/icons/

    pixmap.files = Resources/*
    pixmap.path = $$DATADIR/luckey/hexon/

    desktop.files = hexon.desktop
    desktop.path = $$DATADIR/applications/

    appdata.files = hexon.appdata.xml
    appdata.path = $$DATADIR/appdata/

    INSTALLS += target icon desktop appdata
}

DISTFILES += \
    LICENSE_TEMPLATE
