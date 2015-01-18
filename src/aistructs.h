#ifndef AI_STRUCTS_H
#define AI_STRUCTS_H

#include <QList>
#include <QDebug>


namespace KSquares
{
	enum ChainType {CHAIN_SHORT, CHAIN_LONG, CHAIN_LOOP, CHAIN_UNKNOWN};
	typedef struct Chain_t
	{
		QList<int> lines;
		QList<int> squares;
		ChainType type;
		bool ownChain;
	} Chain;
	
	// used to convey connections to another square via a line
	typedef struct LSConnection_t
	{
		LSConnection_t(int l, int s) {line = l; square = s;}
		int line;
		int square;
		friend QDebug operator<<(QDebug dbg, const KSquares::LSConnection_t &con) { dbg.nospace() << "LSConnection(l: " << con.line << ", s: " << con.square << ")"; return dbg.maybeSpace(); }
	} LSConnection;
	
	typedef struct BoardAnalysis_t
	{
		QList<KSquares::Chain> chains;
		QList<KSquares::Chain> chainsAfterCapture;
		
		// list of indices of chains
		QList<int> capturableLongChains;
		QList<int> capturableLoopChains;
		QList<int> capturableShortChains;
		
		// list of indices of chainsAfterCapture
		QList<int> openLongChains;
		QList<int> openLoopChains;
		QList<int> openShortChains;
	} BoardAnalysis;
}


#endif
