cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE( Qt4 REQUIRED QT_USE_QT* )

INCLUDE( ${QT_USE_FILE} )

set( ksquarestest_SRCS 
	regression.cpp
	../src/ksquaresgame.cpp
	../src/aicontroller.cpp
	../src/aifunctions.cpp
	../src/ksquaresio.cpp
	../src/board.cpp
)

kde4_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

include_directories( ../src )

configure_file( testboardpath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/testboardpath.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated/ )

add_definitions( -DTESTBOARDPATH="${CMAKE_CURRENT_SOURCE_DIR}/boards")

message("TESTBOARDPATH = " ${TESTBOARDPATH})

kde4_add_unit_test( ksquarestest
  TESTNAME regression-ksquarestest
  ${ksquarestest_SRCS}
)

target_link_libraries( ksquarestest
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)
