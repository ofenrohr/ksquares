//
// Created by ofenrohr on 22.07.18.
//

#include "AlphaDotsHelper.h"

#include <QtCore/QDir>
#include "settings.h"

bool AlphaDotsHelper::alphaDotsConfigurationOK() {
    QDir dir(Settings::alphaDotsDir());
    return dir.exists();
    // TODO: check python config
    // TODO: check if we can actually run scripts
}
