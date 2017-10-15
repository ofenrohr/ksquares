//
// Created by ofenrohr on 10/15/17.
//

#ifndef KSQUARES_MLDATAGENERATOR_H
#define KSQUARES_MLDATAGENERATOR_H


#include <QtCore/QArgument>
#include <KXmlGuiWindow>
#include <QtWidgets/QLabel>

class MLDataGenerator : public KXmlGuiWindow
{
    Q_OBJECT

public:
    ///Constructor
    MLDataGenerator();
    ~MLDataGenerator();

    void initObject();
    QImage generateImage();

private:
    QLabel *m_view;
};


#endif //KSQUARES_MLDATAGENERATOR_H
