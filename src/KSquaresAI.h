//
// Created by ofenrohr on 24.07.18.
//

#ifndef KSQUARES_KSQUARESAI_H
#define KSQUARES_KSQUARESAI_H

class KSquaresAi : public aiFunctions, public BoardAnalysisFunctions
{
	public:
		typedef QSharedPointer<KSquaresAi> Ptr;
		KSquaresAi(int w, int h) : aiFunctions(w, h) {}
		virtual ~KSquaresAi() {}
		// call constructor with width, height, playerId, aiLevel
		//virtual ~KSquaresAi() = 0;
		virtual int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory) = 0;
		virtual QString getName() = 0;
		virtual bool enabled() { qDebug() << "KSquaresAI -> enabled"; return true; }
		virtual bool tainted() { return false; } // used for error signaling with external ai
		virtual long lastMoveTime() { return -3; } // time used to calculate move
		virtual int crashCount() { return 0; } // times the ai crashed and could be recovered
};

#endif //KSQUARES_KSQUARESAI_H
