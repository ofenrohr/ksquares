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
	../src/aiHeuristic.cpp
	../src/aiAlphaBeta.cpp
	../src/aistructs.cpp
)

kde4_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

include_directories( ../src )


set(TESTBOARDPATH "${CMAKE_CURRENT_SOURCE_DIR}/boards")

configure_file( testboardpath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/testboardpath.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated/ )
link_libraries(
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)

kde4_add_unit_test( test-hardai
  TESTNAME hardai
  ${ksquarestest_SRCS}
  hardAi.cpp
)

kde4_add_unit_test( test-middleai
  TESTNAME middleai
  ${ksquarestest_SRCS}
  middleAi.cpp
)

kde4_add_unit_test( test-load
  TESTNAME load
  ${ksquarestest_SRCS}
  load.cpp
)

kde4_add_unit_test( test-refactor
  TESTNAME refactor
  ${ksquarestest_SRCS}
  refactor.cpp
)

kde4_add_unit_test( test-minimax
  TESTNAME minimax
  ${ksquarestest_SRCS}
  minimax.cpp
)

kde4_add_unit_test( test-aiboard
  TESTNAME aiboard
  ${ksquarestest_SRCS}
  aiboard.cpp
)

kde4_add_unit_test( test-berlekamp
  TESTNAME berlekamp
  ${ksquarestest_SRCS}
  berlekamp.cpp
)

kde4_add_unit_test( test-alphabeta
  TESTNAME alphabeta
  ${ksquarestest_SRCS}
  alphabeta.cpp
)
