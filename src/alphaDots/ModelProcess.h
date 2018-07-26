//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_MODELPROCESS_H
#define KSQUARES_MODELPROCESS_H


#include <QtCore/QString>
#include "ExternalProcess.h"

namespace AlphaDots {
    /**
     * Holds all data about a ExternalProcess that runs a alpha dots model
     */
    class ModelProcess : public QObject {
        Q_OBJECT
    public:
        typedef QSharedPointer<ModelProcess> Ptr;

        /**
         * Create a model server process for a given model configuration
         * @param model The name of the alpha dots model
         * @param boxesWidth Width of the board in boxes
         * @param boxesHeight Height of the board in boxes
         * @param port The port the model server should listen on
         * @param allowGPU Disable GPU restriction by environment variables
         * @param debug Set debug flag
         * @param logdest log file destination directory
         */
        ModelProcess(QString model, int boxesWidth, int boxesHeight, int port, QString modelKey);
        ~ModelProcess();

        QString model();
        int width();
        int height();
        int port();
        QString key();

    private:
        QString _model;
        int _width;
        int _height;
        int _modelPort;
        QString _processKey;
    };
}


#endif //KSQUARES_MODELPROCESS_H
