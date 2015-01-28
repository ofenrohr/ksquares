#ifndef AI_STRUCTS_H
#define AI_STRUCTS_H

#include <QList>
#include <QDebug>


namespace KSquares
{
	enum ChainType {CHAIN_SHORT, CHAIN_LONG, CHAIN_LOOP, CHAIN_SPECIAL, CHAIN_UNKNOWN};
	typedef struct Chain_t
	{
		QList<int> lines;
		QList<int> squares;
		ChainType type;
		bool ownChain;
		friend QDebug operator<<(QDebug dbg, const KSquares::Chain_t &c) { dbg.nospace() << "Chain(lines: " << c.lines << ", squares: " << c.squares << ", type: " << c.type << ", own: " << c.ownChain << ")"; return dbg.maybeSpace(); }
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
		// capturable chains
		QList<KSquares::Chain> chains;
		// chains after capture of other chains
		QList<KSquares::Chain> chainsAfterCapture;
		
		// list of indices of chains
		QList<int> capturableLongChains;
		QList<int> capturableLoopChains;
		QList<int> capturableShortChains;
		
		// list of indices of chainsAfterCapture
		QList<int> openLongChains;
		QList<int> openLoopChains;
		QList<int> openShortChains;
		
		// lines that have a special impact on the game:
		// * lines that create a loop chain when drawn (cycles that have a connection to ground)
		QList<int> specialLines;
		// safe moves
		QList<int> safeLines;
		
		friend QDebug operator<<(QDebug dbg, const KSquares::BoardAnalysis_t &a)
		{
			dbg.nospace() << "BoardAnalysis\n";
			dbg.nospace() << "|-> chains: " << a.chains << "\n";
			dbg.nospace() << "|-> chainsAfterCapture: " << a.chainsAfterCapture << "\n"; 
			dbg.nospace() << "|-> capturableLongChains: " << a.capturableLongChains << "\n";
			dbg.nospace() << "|-> capturableLoopChains: " << a.capturableLoopChains << "\n";
			dbg.nospace() << "|-> capturableShortChains: " << a.capturableShortChains << "\n";
			dbg.nospace() << "|-> openLongChains: " << a.openLongChains << "\n";
			dbg.nospace() << "|-> openLoopChains: " << a.openLoopChains << "\n";
			dbg.nospace() << "|-> openShortChains: " << a.openShortChains << "\n";
			dbg.nospace() << "|-> specialLines: " << a.specialLines << "\n";
			
			return dbg.maybeSpace();
		}
	} BoardAnalysis;
}


#endif
