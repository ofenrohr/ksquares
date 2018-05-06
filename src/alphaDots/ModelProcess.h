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
    class ModelProcess {
    public:
        typedef QSharedPointer<ModelProcess> Ptr;

        /**
         * Create a model server process for a given model configuration
         * @param model The name of the alpha dots model
         * @param boxesWidth Width of the board in boxes
         * @param boxesHeight Height of the board in boxes
         * @param port The port the model server should listen on
         * @param allowGPU Disable GPU restriction by environment variables
         */
        ModelProcess(QString model, int boxesWidth, int boxesHeight, int port, bool allowGPU);
        ~ModelProcess();

        bool isRunning();
        int port();
    private:
        ExternalProcess *modelServer;
        int width;
        int height;
        int modelPort;
    };
}


#endif //KSQUARES_MODELPROCESS_H
