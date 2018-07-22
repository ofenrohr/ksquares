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
    /*
    connect(playerOneAiConfig, static_cast<void (QToolButton::*)(QAction*)>(&QToolButton::triggered), this, &NewGameDialog::playerOneAiConfigSlot);
    connect(playerTwoAiConfig, static_cast<void (QToolButton::*)(QAction*)>(&QToolButton::triggered), this, &NewGameDialog::playerTwoAiConfigSlot);
    connect(playerThreeAiConfig, static_cast<void (QToolButton::*)(QAction*)>(&QToolButton::triggered), this, &NewGameDialog::playerThreeAiConfigSlot);
    connect(playerFourAiConfig, static_cast<void (QToolButton::*)(QAction*)>(&QToolButton::triggered), this, &NewGameDialog::playerFourAiConfigSlot);
     */
    connect(playerOneAiConfig, SIGNAL(clicked()), this, SLOT(playerOneAiConfigSlot()));
    connect(playerTwoAiConfig, SIGNAL(clicked()), this, SLOT(playerTwoAiConfigSlot()));
    connect(playerThreeAiConfig, SIGNAL(clicked()), this, SLOT(playerThreeAiConfigSlot()));
    connect(playerFourAiConfig, SIGNAL(clicked()), this, SLOT(playerFourAiConfigSlot()));

    adjustEnabledUsers(spinNumOfPlayers->value());

    updateModelList();
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

void NewGameDialog::updateModelList() {
    if (!AlphaDotsHelper::alphaDotsConfigurationOK()) {
        QMessageBox::warning(this, tr("AlphaDots not configured"), tr("Please configure AlphaDots in Settings -> Computer player"));
        aiModel->clear();
        return;
    }
    QList<AlphaDots::ModelInfo> models = AlphaDots::ProtobufConnector::getInstance().getModelList();

    aiModel->clear();
    for (auto model : models) {
        qDebug() << "model " << model.name();
        aiModel->addItem(model.name());
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
    switch (ai) {
        case KSquares::AI_CONVNET: {
            auto *convnetDialog = new ConvNetAISettingsDialog();

            QList<AlphaDots::ModelInfo> models = getModels(ai);

            // check if there are models
            if (models.empty()) {
                QMessageBox::warning(this, tr("AlphaDots Error"),
                                     tr("There are no models for the ConvNet AI. Check your AlphaDots installation"));
                convnetDialog->deleteLater();
                break;
            }

            QString initiallySelectedModel = Settings::aiConvNetModels()[player];

            // fill model list
            convnetDialog->aiModel->clear();
            for (const auto &model : models) {
                convnetDialog->aiModel->addItem(model.name());
                if (model.name() == initiallySelectedModel) {
                    convnetDialog->aiModel->setCurrentText(model.name());
                }
            }

            // execute the dialog
            if (!convnetDialog->exec() == QDialog::Accepted) {
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
            alphazeroDialog->exec();
            alphazeroDialog->deleteLater();
            break;
        }
        default:
            QMessageBox::information(this, tr("No special settings"), tr("There are no special settings for this ai"));
    }
}
