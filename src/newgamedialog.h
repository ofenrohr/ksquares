/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H

#include "ui_newgamewidget.h"
#include <QDialog>
#include <alphaDots/ModelInfo.h>

class NewGameDialog : public QDialog, public Ui::NewGameWidget
{
    Q_OBJECT
public:
    explicit NewGameDialog(QWidget *parent = 0);

public slots:
    void adjustEnabledUsers(int);
    void updateModelList();
    void playerOneAiConfigSlot();
    void playerTwoAiConfigSlot();
    void playerThreeAiConfigSlot();
    void playerFourAiConfigSlot();

private:
    void aiConfigDialog(int player, int ai);
    QList<AlphaDots::ModelInfo> getModels(int ai);
};

#endif // NEWGAMEDIALOG_H
