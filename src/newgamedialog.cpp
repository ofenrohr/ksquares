/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "newgamedialog.h"
#include "aiLevelList.h"
#include "MCTSAlphaZeroAISettingsDialog.h"
#include "ConvNetAISettingsDialog.h"
#include "AlphaDotsHelper.h"
#include "aiAlphaZeroMCTS.h"
#include <QDebug>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/ModelInfo.h>
#include <settings.h>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QAction>

NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent)
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    setupUi(mainWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewGameDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewGameDialog::reject);
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    setWindowTitle(tr("New Game"));
    connect(spinNumOfPlayers, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewGameDialog::adjustEnabledUsers);
    connect(playerOneAiConfig, SIGNAL(clicked()), this, SLOT(playerOneAiConfigSlot()));
    connect(playerTwoAiConfig, SIGNAL(clicked()), this, SLOT(playerTwoAiConfigSlot()));
    connect(playerThreeAiConfig, SIGNAL(clicked()), this, SLOT(playerThreeAiConfigSlot()));
    connect(playerFourAiConfig, SIGNAL(clicked()), this, SLOT(playerFourAiConfigSlot()));

    adjustEnabledUsers(spinNumOfPlayers->value());
}

void NewGameDialog::adjustEnabledUsers(int numOfPlayers)
{
    switch (numOfPlayers) {
    case 2:
        labelPlayer3Name->setEnabled(false);
        playerThreeName->setEnabled(false);
        playerThreeHuman->setEnabled(false);
    // no break!
    case 3:
        labelPlayer4Name->setEnabled(false);
        playerFourName->setEnabled(false);
        playerFourHuman->setEnabled(false);
    case 4:
        break;
    default:
        qCritical() << "NewGameDialog::adjustEnabledUsers(): numOfPlayers out of range!!";
    }

    switch (numOfPlayers) {
    case 4:
        labelPlayer4Name->setEnabled(true);
        playerFourName->setEnabled(true);
        playerFourHuman->setEnabled(true);
    case 3:
        labelPlayer3Name->setEnabled(true);
        playerThreeName->setEnabled(true);
        playerThreeHuman->setEnabled(true);
    case 2:
    default:
        break;
    }
}

QList<AlphaDots::ModelInfo> NewGameDialog::getModels(int ai) {
    if (!AlphaDotsHelper::alphaDotsConfigurationOK()) {
        QMessageBox::warning(this, tr("AlphaDots not configured"), tr("Please configure AlphaDots in Settings -> Computer player"));
        return QList<AlphaDots::ModelInfo>();
    }
    QList<AlphaDots::ModelInfo> allModels = AlphaDots::ProtobufConnector::getInstance().getModelList();
    QList<AlphaDots::ModelInfo> ret;
    for (auto model : allModels) {
        if (model.ai() == tr("MCTS-AlphaZero") && ai == KSquares::AI_MCTS_ALPHAZERO) {
            ret.append(model);
        }
        if (model.ai() == tr("ConvNet") && ai == KSquares::AI_CONVNET) {
            ret.append(model);
        }
    }
    return ret;
}

void NewGameDialog::playerOneAiConfigSlot() {
    qDebug() << "playerOneAiConfigSlot";
    aiConfigDialog(0, playerOneAiLevel->currentIndex());
}

void NewGameDialog::playerTwoAiConfigSlot() {
    qDebug() << "playerTwoAiConfigSlot";
    aiConfigDialog(1, playerTwoAiLevel->currentIndex());
}

void NewGameDialog::playerThreeAiConfigSlot() {
    qDebug() << "playerThreeAiConfigSlot";
    aiConfigDialog(2, playerThreeAiLevel->currentIndex());
}

void NewGameDialog::playerFourAiConfigSlot() {
    qDebug() << "playerFourAiConfigSlot";
    aiConfigDialog(3, playerFourAiLevel->currentIndex());
}

void NewGameDialog::aiConfigDialog(int player, int ai) {
    // only convnet and alphazero get special settings.
    if (ai != KSquares::AI_CONVNET && ai != KSquares::AI_MCTS_ALPHAZERO) {
        QMessageBox::information(this, tr("No special settings"), tr("There are no special settings for this AI"));
        return;
    }

    // get the model list for ai
    QList<AlphaDots::ModelInfo> models = getModels(ai);

    // check if there are models
    if (models.empty()) {
        QMessageBox::warning(this, tr("AlphaDots Error"),
                             tr("There are no models. Check your AlphaDots installation"));
    }

    switch (ai) {
        case KSquares::AI_CONVNET: {
            auto *convnetDialog = new ConvNetAISettingsDialog();

            convnetDialog->setWindowTitle(tr("ConvNet AI Settings"));

            // fill model list
            convnetDialog->aiModel->clear();
            for (const auto &model : models) {
                convnetDialog->aiModel->addItem(model.name());
            }

            // preselect configured model
            QString initiallySelectedModel = Settings::aiConvNetModels()[player];
            convnetDialog->aiModel->setCurrentText(initiallySelectedModel);

            // execute the dialog
            if (convnetDialog->exec() == QDialog::Rejected) {
                convnetDialog->deleteLater();
                break;
            }

            // store dialog result
            QString selectedModel = convnetDialog->aiModel->currentText();
            QStringList convNetModels = Settings::aiConvNetModels();
            if (convNetModels.length() != 4) {
                qDebug() << "Setting aIConvNetModels does not have 4 entries. Resetting...";
                convNetModels.clear();
                convNetModels.append(selectedModel);
            }
            convNetModels[player] = selectedModel;
            Settings::setAiConvNetModels(convNetModels);

            // save settings
            Settings::self()->save();

            convnetDialog->deleteLater();
            break;
        }
        case KSquares::AI_MCTS_ALPHAZERO: {
            auto *alphazeroDialog = new MCTSAlphaZeroAISettingsDialog();

            alphazeroDialog->setWindowTitle(tr("MCTS Alpha Zero AI Settings"));

            // fill model list
            alphazeroDialog->aiModel->clear();
            for (const auto &model : models) {
                alphazeroDialog->aiModel->addItem(model.name());
            }

            // preselect configured model
            QString initiallySelectedModel = Settings::aiMCTSAlphaZeroModels()[player];
            alphazeroDialog->aiModel->setCurrentText(initiallySelectedModel);

            // set the AlphaZero hyperparameters
            alphazeroDialog->mctsIterations->setValue(AlphaDots::aiAlphaZeroMCTS::mcts_iterations);
            alphazeroDialog->cPUCT->setValue(AlphaDots::aiAlphaZeroMCTS::C_puct);
            alphazeroDialog->dirichletNoise->setValue(AlphaDots::aiAlphaZeroMCTS::dirichlet_alpha);

            // execute the dialog
            if (alphazeroDialog->exec() == QDialog::Rejected) {
                alphazeroDialog->deleteLater();
                break;
            }

            // store dialog result
            QString selectedModel = alphazeroDialog->aiModel->currentText();
            QStringList mctsModels = Settings::aiMCTSAlphaZeroModels();
            if (mctsModels.length() != 4) {
                qDebug() << "Setting aiMCTSAlphaZeroModels does not have 4 entries. Resetting...";
                mctsModels.clear();
                mctsModels.append(selectedModel);
            }
            mctsModels[player] = selectedModel;
            Settings::setAiMCTSAlphaZeroModels(mctsModels);
            AlphaDots::aiAlphaZeroMCTS::mcts_iterations = alphazeroDialog->mctsIterations->value();
            AlphaDots::aiAlphaZeroMCTS::C_puct = alphazeroDialog->cPUCT->value();
            AlphaDots::aiAlphaZeroMCTS::dirichlet_alpha = alphazeroDialog->dirichletNoise->value();

            // save settings
            Settings::self()->save();

            alphazeroDialog->deleteLater();
            break;
        }
        default:
            QMessageBox::information(this, tr("No special settings"), tr("There are no special settings for this ai"));
    }
}
