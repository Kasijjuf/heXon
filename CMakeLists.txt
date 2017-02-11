project( hexon )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( 
	-Wall
	-std=c++14
	-O2
	)
include_directories ( 
    ../heXon/Urho3D/include
    ../heXon/Urho3D/include/Urho3D/ThirdParty
	)
set ( hexon_HDRS
	apple.h
	bubble.h
	bullet.h
	chaoball.h
	chaoflash.h
	chaomine.h
	chaozap.h
	controllable.h
	door.h
	effect.h
	effectmaster.h
	enemy.h
	explosion.h
	flash.h
	heart.h
	hexocam.h
	hitfx.h
	inputmaster.h
	line.h
	luckey.h
	mastercontrol.h
	muzzle.h
	pickup.h
	player.h
	razor.h
	sceneobject.h
	seeker.h
	spawnmaster.h
	spire.h
	splatterpillar.h
	TailGenerator.h
	tile.h
	pilot.h
	phaser.h
	arena.h
	lobby.h
	highest.h
	ship.h
	hexonevents.h
	gui3d.h
	panel.h
	coin.h
	effectinstance.h
	)

set ( hexon_SRCS
	apple.cpp
	bubble.cpp
	bullet.cpp
	chaoball.cpp
	chaoflash.cpp
	chaomine.cpp
	chaozap.cpp
	controllable.cpp
	door.cpp
	effect.cpp
	effectmaster.cpp
	enemy.cpp
	explosion.cpp
	flash.cpp
	heart.cpp
	hexocam.cpp
	hitfx.cpp
	inputmaster.cpp
	line.cpp
	luckey.cpp
	mastercontrol.cpp
	muzzle.cpp
	pickup.cpp
	player.cpp
	razor.cpp
	sceneobject.cpp
	seeker.cpp
	spawnmaster.cpp
	spire.cpp
	splatterpillar.cpp
	TailGenerator.cpp
	tile.cpp
	pilot.cpp
	phaser.cpp
	arena.cpp
	lobby.cpp
	highest.cpp
	ship.cpp
	gui3d.cpp
	panel.cpp
	coin.cpp
	effectinstance.cpp
	)

add_executable ( hexon ${hexon_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( hexon )
