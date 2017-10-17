//
// Created by ofenrohr on 10/15/17.
//

#ifndef KSQUARES_MLDATAGENERATOR_H
#define KSQUARES_MLDATAGENERATOR_H


#include <QtCore/QArgument>
#include <KXmlGuiWindow>
#include <QtWidgets/QLabel>
#include "aiBoard.h"

class MLDataGenerator : public KXmlGuiWindow
{
    Q_OBJECT

public:
    ///Constructor
    MLDataGenerator();
    ~MLDataGenerator();

    void initObject();

    /**
     * Generates board with random (usrful) state with some safe moves left
     * @param width in boxes
     * @param height in boxes
     * @param safeMoves number of safe lines left
     * @return board with requested features
     */
    static aiBoard::Ptr generateRandomBoard(int width, int height, int safeMoves);
    static QImage generateImage();

private:
    QLabel *m_view;
};


#endif //KSQUARES_MLDATAGENERATOR_H
