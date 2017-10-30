//
// Created by ofenrohr on 26.10.17.
//

#ifndef KSQUARES_AICONVNET_H
#define KSQUARES_AICONVNET_H

#include <string>
#include "aicontroller.h"
#include <zmq.hpp>
#include "alphaDots/DotsAndBoxesImage.pb.h"


class aiConvNet : public KSquaresAi {
    public:
        aiConvNet(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~aiConvNet();

		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return QStringLiteral("convnet"); }
		virtual bool enabled() { return true; }
		virtual bool tainted() { return false; }
		virtual long lastMoveTime() { return turnTime; }

    protected:
		/// The ID of the player this AI belongs to
		int playerId;
		/// number of players - 1
		int maxPlayerId;
		/// board width in squares
		int width;
		/// board height in squares
		int height;
		/// The strength of the ai
		int level;
		/// time logging
		long turnTime;
};


#endif //KSQUARES_AICONVNET_H
