ENABLE_TESTING()
cmake_minimum_required(VERSION 2.8)

find_package(GSL REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

file(GLOB protoMessages ../src/alphaDots/protobuf/*.proto)
PROTOBUF_GENERATE_CPP(ProtoSrcs ProtoHeaders ${protoMessages})

file(GLOB alphaDotsSRCS ../src/alphaDots/*.cpp)
file(GLOB alphaDotsModelEvaluationSRCS ../src/alphaDots/modelEvaluation/*.cpp)
file(GLOB alphaDotsDatasetsSRCS ../src/alphaDots/datasets/*.cpp)
file(GLOB alphaDotsAiEvalSRCS ../src/alphaDots/selfPlay/*.cpp)
file(GLOB alphaDotsGamplayAnalysisSRCS ../src/alphaDots/modelEvaluation/gameplayAnalysis/*.cpp)

message("blah")
message(${alphaDotsAiEvalSRCS})
message("blah")

set( ksquarestest_SRCS 
        ../src/ksquaresgame.cpp
		../src/gameboardscene.cpp
		../src/gameboardview.cpp
		../src/highlightanimation.cpp
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
		../src/dbgame-nohash.cpp
		../src/aiDabbleNative.cpp
		../src/aiConvNet.cpp
		../src/AlphaZeroMCTSNode.cpp
		../src/aiAlphaZeroMCTS.cpp
        ../src/MCTSAlphaZeroAISettingsDialog.cpp
		../src/aiLevelList.h
		../src/ConvNetAISettingsDialog.cpp
		../src/AlphaDotsHelper.cpp
		../src/aiControllerWorker.cpp
		../src/KSquaresAI.h
        ${ProtoSrcs}
        ${alphaDotsSRCS}
		${alphaDotsModelEvaluationSRCS}
		${alphaDotsDatasetsSRCS}
		${alphaDotsAiEvalSRCS}
		${alphaDotsGamplayAnalysisSRCS}
        )

kconfig_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC ../src/settings.kcfgc)

#kde4_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S
kconfig_add_kcfg_files(ksquarestest_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

include_directories( ../src ${QJSON_INCLUDE_DIR} ${ZeroMQ_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} )


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
		Qt5::Core
        Qt5::Test
        KF5KDEGames
        #${QJSON_LIBRARIES}
		${ZeroMQ_LIBRARY}
		${PROTOBUF_LIBRARY}
		${ZLIB_LIBRARIES}
		GSL::gsl
)


add_executable(ksquarestest
		main.cpp
		testutils.cpp
        ${ksquarestest_SRCS}
        hardAi.cpp
        berlekamp.cpp
		aiboard.cpp
		alphazero.cpp
		GSLTest.cpp
		SharedPointerTest.cpp
)
#add_test(ksquarestest ksquarestest-hardai)

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
