//
// Created by ofenrohr on 22.07.18.
//

#ifndef KSQUARES_CONVNETAISETTINGSDIALOG_H
#define KSQUARES_CONVNETAISETTINGSDIALOG_H

#include "ui_convnetaisettings.h"

#include <QtWidgets/QDialog>

class ConvNetAISettingsDialog : public QDialog, public Ui::AiConvNetConfigDialog {
Q_OBJECT
public:
    explicit ConvNetAISettingsDialog(QWidget *parent = 0);
};


#endif //KSQUARES_CONVNETAISETTINGSDIALOG_H
