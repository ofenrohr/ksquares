//
// Created by ofenrohr on 22.07.18.
//

#include "ConvNetAISettingsDialog.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

ConvNetAISettingsDialog::ConvNetAISettingsDialog(QWidget *parent) {
    QWidget *mainWidget = new QWidget(parent);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    setupUi(mainWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConvNetAISettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConvNetAISettingsDialog::reject);
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
}

