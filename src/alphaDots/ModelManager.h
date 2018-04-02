//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_MODELMANAGER_H
#define KSQUARES_MODELMANAGER_H


#include <QtCore/QString>
#include <QtCore/QMap>
#include <qdebug.h>
#include "ModelProcess.h"

namespace AlphaDots {
    /**
     * Handles alpha dots models. Starts python processes as required.
     */
    class ModelManager {
    public:
        static ModelManager& getInstance() {
            static ModelManager instance;
            qDebug() << "[ModelManager] getInstance";
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

    private:
        ModelManager() = default;

        QMap<QString, ModelProcess::Ptr> processMap;
        int port = 12354;

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

        void sleep(int ms);
    };
}


#endif //KSQUARES_MODELMANAGER_H
