ENABLE_TESTING()
cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE( Qt4 REQUIRED QT_USE_QT* )

INCLUDE( ${QT_USE_FILE} )

set( ksquarestest_SRCS 
	../src/ksquaresgame.cpp
	../src/aicontroller.cpp
	../src/aifunctions.cpp
	../src/ksquaresio.cpp
	../src/board.cpp
	../src/aiEasyMediumHard.cpp
	../src/aiMiniMax.cpp
	../src/aiBoard.cpp
)

kde4_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

include_directories( ../src )


set(TESTBOARDPATH "${CMAKE_CURRENT_SOURCE_DIR}/boards")

configure_file( testboardpath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/testboardpath.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated/ )


kde4_add_unit_test( test-hardai
  TESTNAME hardai
  ${ksquarestest_SRCS}
  hardAi.cpp
)
target_link_libraries( test-hardai
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)

kde4_add_unit_test( test-middleai
  TESTNAME middleai
  ${ksquarestest_SRCS}
  middleAi.cpp
)
target_link_libraries( test-middleai
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)

kde4_add_unit_test( test-load
  TESTNAME load
  ${ksquarestest_SRCS}
  load.cpp
)
target_link_libraries( test-load
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)

kde4_add_unit_test( test-refactor
  TESTNAME refactor
  ${ksquarestest_SRCS}
  refactor.cpp
)
target_link_libraries( test-refactor
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)

kde4_add_unit_test( test-minimax
  TESTNAME minimax
  ${ksquarestest_SRCS}
  minimax.cpp
)
target_link_libraries( test-minimax
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)
