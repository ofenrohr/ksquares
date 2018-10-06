//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_MODELEVALUATION_H
#define KSQUARES_MODELEVALUATION_H


#include <KXmlGui/KXmlGuiWindow>
#include <gameboardscene.h>
#include <ksquaresgame.h>
#include <aicontroller.h>
#include <QtCore/QDateTime>
#include <alphaDots/ReportLogger.h>

#include "TestResultModel.h"
#include "FastModelEvaluation.h"
#include "ui_ModelEvaluation.h"

namespace AlphaDots {
    class ModelEvaluation : public KXmlGuiWindow, public Ui::ModelEvaluationForm {
    Q_OBJECT
    public:
        /**
         * Controller for model evaluation.
         * @param models Empty string to evaluate all available models.
         * @param opponentModels List of opponent models, empty string for all models
         * @param fast Run fast multi-threaded evaluation
         */
        explicit ModelEvaluation(QString &models, QString &opponentModels, bool fast=false, int threadCnt=4,
                int games=10, QPoint boardSize=QPoint(5,5), bool quickStart=true, QString reportDir="");

		~ModelEvaluation() override;

		QList<ModelInfo> getModelList(QString &models);
        void initObject();

		static void printModelList();

		static void createTestSetups(QList<AITestSetup> &testSetups, QPoint boardSize, int timeout,
				QList<ModelInfo> &modelList, QList<ModelInfo> &opponentModelList, int gamesPerAi);

		static void writeResultsToReport(ReportLogger::Ptr &report, QDateTime &startTime, QDateTime &endTime,
        TestResultModel *resultModel, int threads, bool evaluationRunning, QList<ModelInfo> &modelList,
        QList<ModelInfo> &opponentModelList, bool includeArgs = true, bool includeGIT = true);

    public slots:
		void aiChoseLine(const int &line);
		void nextGame();
		void saveResultsAs();
		void evaluationFinished();

	private slots:
		void aiChooseLine();
		void playerTakeTurn(KSquaresPlayer* currentPlayer);
		void gameOver(const QVector<KSquaresPlayer> & /*playerList*/);

    private:
        QDateTime startTime;
		QDateTime endTime;
		bool evaluationRunning;

		QList<AITestSetup> testSetups;
        AITestSetup currentSetup;

        TestResultModel *resultModel;
        QList<ModelInfo> modelList;
		QList<ModelInfo> opponentModelList;

        bool fastEvaluation;
		FastModelEvaluation *fastEvaluationHandler;

		int gamesPerAi;
        int threads;
        bool quickStart;
        QString reportDir;

        QWidget *m_view;
        GameBoardScene *m_scene;
		KSquaresGame* sGame;
		QList<aiController::Ptr> aiList;
		QThread* thread;

        QList<int> lineLog;
        QList<int> autoFillLines;

		void createTestSetups(QPoint boardSize);
		/**
		 * Loads a slow GUI test setup.
		 * @param setup
		 */
        void loadTestSetup(const AITestSetup &setup);
		/**
		 * Get the name of a ai. This model evaluation uses its own numbering scheme:
		 * 0,1,2 = Easy, Medium, Hard
		 * 3,...,n = Models
		 * @param level model evaluation ai level
		 * @return ai name
		 */
		QString aiName(int level);
    };
}


#endif //KSQUARES_MODELEVALUATION_H
