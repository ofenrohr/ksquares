//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_MODELEVALUATION_H
#define KSQUARES_MODELEVALUATION_H


#include <KXmlGui/KXmlGuiWindow>
#include <gameboardscene.h>
#include <ksquaresgame.h>
#include <aicontroller.h>

#include "ui_ModelEvaluation.h"
#include "TestResultModel.h"

namespace AlphaDots {
    class ModelEvaluation : public KXmlGuiWindow, public Ui::ModelEvaluationForm {
    Q_OBJECT
    public:
        ModelEvaluation(QString models = QStringLiteral(""));
        ~ModelEvaluation();

		QList<ModelInfo> getModelList(QString models);
        void initObject();

		static void printModelList();

	public slots:
		void aiChoseLine(const int &line);
		void nextGame();

	private slots:
		void aiChooseLine();
		void playerTakeTurn(KSquaresPlayer* currentPlayer);
		void gameOver(const QVector<KSquaresPlayer> & /*playerList*/);

    private:
		QString modelsStr;
        QList<AITestSetup> testSetups;
        AITestSetup currentSetup;

        TestResultModel *resultModel;
        QList<ModelInfo> modelList;

        QWidget *m_view;
        GameBoardScene *m_scene;
		KSquaresGame* sGame;
		QList<aiController::Ptr> aiList;
		QThread* thread;

        QList<int> lineLog;

		void createTestSetups();
        void loadTestSetup(const AITestSetup &setup);
		QString aiName(int level);
    };
}


#endif //KSQUARES_MODELEVALUATION_H
