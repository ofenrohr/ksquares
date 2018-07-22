//
// Created by ofenrohr on 22.07.18.
//

#ifndef KSQUARES_MCTSALPHAZEROAISETTINGSDIALOG_H
#define KSQUARES_MCTSALPHAZEROAISETTINGSDIALOG_H

#include "ui_mctsalphazeroaisettings.h"

#include <QDialog>

class MCTSAlphaZeroAISettingsDialog : public QDialog, public Ui::AiMCTSAlphaZeroConfigDialog {
Q_OBJECT
public:
    explicit MCTSAlphaZeroAISettingsDialog(QWidget *parent = 0);

};


#endif //KSQUARES_MCTSALPHAZEROAISETTINGSDIALOG_H
