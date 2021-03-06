
#add_definitions(-DKSQUARES_ALPHABETA_ITERATIVE_DEEPENING)

#find_package(QJSON REQUIRED)



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
    alphaDots/modelEvaluation/TestResultModel.cpp
    alphaDots/modelEvaluation/FastModelEvaluationWorker.cpp
    alphaDots/modelEvaluation/FastModelEvaluation.cpp
    alphaDots/modelEvaluation/AITestSetupManager.cpp
    alphaDots/ModelManager.cpp
    alphaDots/ModelProcess.cpp
    alphaDots/modelEvaluation/gameplayAnalysis/GameplayAnalysis.cpp
    alphaDots/cnpy.cpp
    alphaDots/datasets/StageTwoDataset.cpp
    aiAlphaZeroMCTS.cpp
    AlphaZeroMCTSNode.cpp
    alphaDots/datasets/StageThreeDataset.cpp
    alphaDots/MLImageGenerator.cpp
    alphaDots/selfPlay/SelfPlay.cpp
    alphaDots/selfPlay/SelfPlayWorker.cpp
    alphaDots/datasets/StageFourDataset.cpp
    alphaDots/ProtobufBatch.cpp
    MCTSAlphaZeroAISettingsDialog.cpp
    aiLevelList.h
    ConvNetAISettingsDialog.cpp
    AlphaDotsHelper.cpp
    aiControllerWorker.cpp
    KSquaresAI.h
	alphaDots/selfPlay/GenerateData.cpp
	alphaDots/selfPlay/TrainNetwork.cpp
	alphaDots/selfPlay/EvaluateNetwork.cpp
	alphaDots/AlphaDotsExceptions.cpp
	alphaDots/ReportLogger.cpp
	alphaDots/modelEvaluation/gameplayAnalysis/DoubleDealingAnalysis.cpp
)

file(GLOB protoMessages alphaDots/protobuf/*.proto)
PROTOBUF_GENERATE_CPP(ProtoSrcs ProtoHeaders ${protoMessages})

message(ProtoSrcs: ${ProtoSrcs})
message(ProtoHeaders: ${ProtoHeaders})

include_directories(${CMAKE_SOURCE_DIR}/libKF5KDEGames/highscore ${ZeroMQ_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})


ki18n_wrap_ui(ksquares_SRCS
		newgamewidget.ui
		scoreswidget.ui
		prefs_ai.ui
		prefs_display.ui
		alphaDots/mldataview.ui
		alphaDots/modelEvaluation/ModelEvaluation.ui
        alphaDots/selfPlay/SelfPlayForm.ui
		convnetaisettings.ui
		mctsalphazeroaisettings.ui
)

qt5_add_resources(ksquares_SRCS ksquares.qrc)

kconfig_add_kcfg_files(ksquares_SRCS GENERATE_MOC settings.kcfgc)

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
    ${ZLIB_LIBRARIES}
    GSL::gsl
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
