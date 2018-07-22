//
// Created by ofenrohr on 22.07.18.
//

#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include "MCTSAlphaZeroAISettingsDialog.h"

MCTSAlphaZeroAISettingsDialog::MCTSAlphaZeroAISettingsDialog(QWidget *parent) {
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    setupUi(mainWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MCTSAlphaZeroAISettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MCTSAlphaZeroAISettingsDialog::reject);
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
}
