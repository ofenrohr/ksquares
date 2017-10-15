ENABLE_TESTING()
cmake_minimum_required(VERSION 2.8)

#FIND_PACKAGE( Qt4 REQUIRED QT_USE_QT* )
#find_package(QJSON REQUIRED)

#INCLUDE( ${QT_USE_FILE} )

set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

set( ksquarestest_SRCS 
	../src/ksquaresgame.cpp
	../src/aicontroller.cpp
	../src/aifunctions.cpp
	../src/ksquaresio.cpp
	../src/board.cpp
	../src/aiEasyMediumHard.cpp
#	../src/aiMiniMax.cpp
	../src/aiBoard.cpp
	../src/aiHeuristic.cpp
	../src/aiAlphaBeta.cpp
	../src/aistructs.cpp
	../src/lineSorter.cpp
	../src/boardAnalysis.cpp
	../src/dbgame.cpp
#	../src/dbgame-nohash.cpp
	../src/qdab.cpp
	../src/knox.cpp
	../src/aiMCTS.cpp
)

#kde4_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S
kconfig_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

include_directories( ../src ${QJSON_INCLUDE_DIR} )


set(TESTBOARDPATH "${CMAKE_CURRENT_SOURCE_DIR}/boards")
set(EXTERNALAIPATH "${CMAKE_CURRENT_SOURCE_DIR}/../aux/")

configure_file( ../src/externalaipath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/externalaipath.h )
configure_file( testboardpath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/testboardpath.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated/ )

#add_executable(ksquarestest ${ksquarestest_SRCS})
link_libraries(ksquarestest
    KF5::Completion
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
    KF5::XmlGui
    Qt5::Test
    KF5KDEGames
	#${QJSON_LIBRARIES}
)

#link_libraries(
#  ${KDE4_KDECORE_LIBS}
#  ${QT_QTTEST_LIBRARY}
#  ${KDE4_KDEUI_LIBS}
#  ${KDE4_KIO_LIBS}
#  ${QJSON_LIBRARIES}
#  kdegames
#)

add_test(ksquarestest-hardai
  ${ksquarestest_SRCS}
  hardAi.cpp
)

#kde4_add_unit_test( test-hardai
#  TESTNAME hardai
#  ${ksquarestest_SRCS}
#  hardAi.cpp
#)
#
#kde4_add_unit_test( test-middleai
#  TESTNAME middleai
#  ${ksquarestest_SRCS}
#  middleAi.cpp
#)
#
#kde4_add_unit_test( test-load
#  TESTNAME load
#  ${ksquarestest_SRCS}
#  load.cpp
#)
#
#kde4_add_unit_test( test-refactor
#  TESTNAME refactor
#  ${ksquarestest_SRCS}
#  refactor.cpp
#)
#
## minimax is deprecated
## kde4_add_unit_test( test-minimax
##   TESTNAME minimax
##   ${ksquarestest_SRCS}
##   minimax.cpp
## )
#
#kde4_add_unit_test( test-aiboard
#  TESTNAME aiboard
#  ${ksquarestest_SRCS}
#  aiboard.cpp
#)
#
#kde4_add_unit_test( test-berlekamp
#  TESTNAME berlekamp
#  ${ksquarestest_SRCS}
#  berlekamp.cpp
#)
#
#kde4_add_unit_test( test-alphabeta
#  TESTNAME alphabeta
#  ${ksquarestest_SRCS}
#  alphabeta.cpp
#)
#
#kde4_add_unit_test( test-dabble
#  TESTNAME dabble
#  ${ksquarestest_SRCS}
#  dabble.cpp
#)
