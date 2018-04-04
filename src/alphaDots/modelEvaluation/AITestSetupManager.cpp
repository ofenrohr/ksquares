//
// Created by ofenrohr on 30.03.18.
//

#include "AITestSetupManager.h"

using namespace AlphaDots;

AITestSetupManager::AITestSetupManager(QList<AITestSetup> *setupList) {
    setups = setupList;
}

AITestSetup AITestSetupManager::popSetup(bool *ok) {
    QMutexLocker locker(&setupListMutex);
    if (setups->isEmpty()) {
        *ok = false;
        return {};
    } else {
        *ok = true;
    }
    return setups->takeFirst();
}
