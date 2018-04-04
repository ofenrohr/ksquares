//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_AITESTSETUPMANAGER_H
#define KSQUARES_AITESTSETUPMANAGER_H


#include <QtCore/QMutex>
#include "AITestSetup.h"

namespace AlphaDots {
    /**
     * Thread safe AITestSetup manager. Saves all test setups and hands out work to worker threads.
     */
    class AITestSetupManager {
    public:
        explicit AITestSetupManager(QList<AITestSetup> *setupList);
        explicit AITestSetupManager(AITestSetupManager &mgr) { setups = mgr.getSetups(); }

        /**
         * Retrieves and removes a setup from the setup list.
         * @param ok is set to true if a valid setup was returned
         * @return test setup
         */
        AITestSetup popSetup(bool *ok);

        QList<AITestSetup>* getSetups() { QMutexLocker locker(&setupListMutex); return setups; }

    private:
        mutable QMutex setupListMutex;
        QList<AITestSetup> *setups;
    };
}


#endif //KSQUARES_AITESTSETUPMANAGER_H
