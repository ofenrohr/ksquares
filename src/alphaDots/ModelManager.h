//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_MODELMANAGER_H
#define KSQUARES_MODELMANAGER_H


#include <QtCore/QString>
#include <QtCore/QMap>
#include <qdebug.h>
#include <QtCore/QMutex>
#include "ModelProcess.h"

namespace AlphaDots {
    /**
     * Handles alpha dots models. Starts python processes as required.
     */
    class ModelManager {
    public:
        static ModelManager& getInstance() {
            static ModelManager instance;
            //qDebug() << "[ModelManager] getInstance";
            return instance;
        }
        ModelManager(ModelManager const &) = delete;
        void operator=(ModelManager const &) = delete;

        /**
         * Ensures that a process for the specified configuration is running and returns the port number or
         * -1 of something failed.
         * @param modelName name of the alpha dots model
         * @param width board width in boxes
         * @param height board height in boxes
         * @return port number of the model server or -1 on error
         */
        int ensureProcessRunning(QString modelName, int width, int height);

        /**
         * Allow the model Process to use gpu resources.
         * @param allowGPU
         */
        void allowGPU(bool allowGPU);

        /**
         * Stops all running model processes
         */
        void stopAll();

        /**
         * Sets the debug mode
         */
        void setDebug(bool mode);

        bool getDebug() { return debug; }

        void setLogDest(QString &dest) {logDest = dest;}

    private:
        ModelManager() = default;

        QMap<QString, ModelProcess::Ptr> processMap;
        int port = 12354;
        QMutex getProcessMutex;

        /**
         * Get the process for the specified configuration
         * @param modelName
         * @param width
         * @param height
         * @return
         */
        ModelProcess::Ptr getProcess(QString modelName, int width, int height);

        /**
         * Convert the model configuration data to a string which will be used as the key in processMap
         * @param modelName
         * @param width
         * @param height
         * @return
         */
        QString modelInfoToStr(QString modelName, int width, int height);

        bool useGPU=false;
        bool debug=false;
        QString logDest;
    };
}


#endif //KSQUARES_MODELMANAGER_H
