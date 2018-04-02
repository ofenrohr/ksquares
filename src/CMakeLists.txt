
#add_definitions(-DKSQUARES_ALPHABETA_ITERATIVE_DEEPENING)

#find_package(QJSON REQUIRED)

set(CMAKE_AUTOUIC ON)

set(ksquares_SRCS
	main.cpp
	ksquareswindow.cpp
	gameboardview.cpp
	gameboardscene.cpp
	ksquaresgame.cpp
	aicontroller.cpp
	newgamedialog.cpp
	ksquaresdemowindow.cpp
	themegraphicsitems.cpp
    highlightanimation.cpp
	scoresdialog.cpp
	aifunctions.cpp
	ksquaresio.cpp
	board.cpp
	dbgame.cpp
	aiEasyMediumHard.cpp
#	aiMiniMax.cpp
	aiBoard.cpp
	aiHeuristic.cpp
	nim.cpp
	aiAlphaBeta.cpp
	aistructs.cpp
	ksquarestestwindow.cpp
	lineSorter.cpp
	boardAnalysis.cpp
	dbgame-nohash.cpp
	qdab.cpp
	knox.cpp
	aiMCTS.cpp
    aiDabbleNative.cpp
    alphaDots/MLDataGenerator.cpp
    alphaDots/MLDataGeneratorWorkerThread.cpp
    aiConvNet.cpp
    alphaDots/ProtobufConnector.cpp
    alphaDots/datasets/FirstTryDataset.cpp
    alphaDots/datasets/DatasetGenerator.cpp
    alphaDots/datasets/StageOneDataset.cpp
    alphaDots/ExternalProcess.cpp
    alphaDots/datasets/Dataset.cpp
    alphaDots/datasets/BasicStrategyDataset.cpp
    alphaDots/datasets/SequenceDataset.cpp
    alphaDots/datasets/TrainingSequenceDataset.cpp
    alphaDots/ModelInfo.cpp
    alphaDots/modelEvaluation/AITestSetup.cpp
    alphaDots/modelEvaluation/AITestResult.cpp
    alphaDots/modelEvaluation/ModelEvaluation.cpp
    alphaDots/modelEvaluation/TestResultModel.cpp alphaDots/modelEvaluation/FastModelEvaluationWorker.cpp alphaDots/modelEvaluation/FastModelEvaluationWorker.h alphaDots/modelEvaluation/FastModelEvaluation.cpp alphaDots/modelEvaluation/FastModelEvaluation.h alphaDots/modelEvaluation/AITestSetupManager.cpp alphaDots/modelEvaluation/AITestSetupManager.h alphaDots/ModelManager.cpp alphaDots/ModelManager.h alphaDots/ModelProcess.cpp alphaDots/ModelProcess.h)

file(GLOB protoMessages alphaDots/protobuf/*.proto)
PROTOBUF_GENERATE_CPP(ProtoSrcs ProtoHeaders ${protoMessages})

message(ProtoSrcs: ${ProtoSrcs})
message(ProtoHeaders: ${ProtoHeaders})

include_directories(${CMAKE_SOURCE_DIR}/libKF5KDEGames/highscore ${ZeroMQ_INCLUDE_DIR})


ki18n_wrap_ui(ksquares_SRCS newgamewidget.ui scoreswidget.ui prefs_ai.ui prefs_display.ui alphaDots/mldataview.ui alphaDots/modelEvaluation/ModelEvaluation.ui)

kconfig_add_kcfg_files(ksquares_SRCS GENERATE_MOC settings.kcfgc) #GENERATE_MOC is not needed but it doesn't work without it for some reason :S

set(CMAKE_AUTOMOC ON)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-ksquares.png")
ecm_add_app_icon(ksquares_SRCS ICONS ${ICONS_SRCS})
add_executable(ksquares ${ksquares_SRCS} ${ProtoSrcs})

target_link_libraries(ksquares
    KF5::Completion
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
    KF5::XmlGui
    KF5KDEGames
	#${QJSON_LIBRARIES}
    ${ZeroMQ_LIBRARY}
	${PROTOBUF_LIBRARY}
)

install(TARGETS ksquares ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# setup where to find external AIs
set(EXTERNALAIPATH "${CMAKE_CURRENT_SOURCE_DIR}/../aux/")
configure_file( externalaipath.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/externalaipath.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated/ )

########### install files ###############

install(PROGRAMS org.kde.ksquares.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.ksquares.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR})
install(FILES ksquares.kcfg  DESTINATION  ${KDE_INSTALL_KCFGDIR})
install(FILES ksquaresui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ksquares)

ecm_install_icons(ICONS
    128-apps-ksquares.png
    16-apps-ksquares.png
    22-apps-ksquares.png
    32-apps-ksquares.png
    48-apps-ksquares.png
    64-apps-ksquares.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
