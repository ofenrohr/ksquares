//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_MODELEVALUATION_H
#define KSQUARES_MODELEVALUATION_H


#include <KXmlGui/KXmlGuiWindow>
#include <gameboardscene.h>
#include <ksquaresgame.h>
#include <aicontroller.h>

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
         * @param fast Run fast multi-threaded evaluation
         */
        explicit ModelEvaluation(QString models, bool fast=false, int threadCnt=4, int games=10);

		~ModelEvaluation() override;

		QList<ModelInfo> getModelList(QString models);
        void initObject();

		static void printModelList();

    public slots:
		void aiChoseLine(const int &line);
		void nextGame();
		void saveResultsAs();

	private slots:
		void aiChooseLine();
		void playerTakeTurn(KSquaresPlayer* currentPlayer);
		void gameOver(const QVector<KSquaresPlayer> & /*playerList*/);

    private:
		QList<AITestSetup> testSetups;
        AITestSetup currentSetup;

        TestResultModel *resultModel;
        QList<ModelInfo> modelList;

        bool fastEvaluation;
		FastModelEvaluation *fastEvaluationHandler;

		int gamesPerAi;
        int threads;

        QWidget *m_view;
        GameBoardScene *m_scene;
		KSquaresGame* sGame;
		QList<aiController::Ptr> aiList;
		QThread* thread;

        QList<int> lineLog;

		void createTestSetups();
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
