//
// Created by ofenrohr on 22.09.18.
//

#ifndef KSQUARES_ALPHADOTSEXCEPTIONS_H
#define KSQUARES_ALPHADOTSEXCEPTIONS_H

#include <exception>

namespace AlphaDots {
    class ModelNotFoundException : public std::exception {};
    class InternalAiException : public std::exception {};
}


#endif //KSQUARES_ALPHADOTSEXCEPTIONS_H
