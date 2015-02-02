//
// dbgame.h
//

#include <stdint.h>
#include <cstddef>
#include <string>
#include <cstdio>

typedef uint8_t __int8;
typedef uint16_t __int16;
typedef uint32_t __int32;

typedef enum {dir_horizontal, dir_vertical, dir_invalid} Direction;

class Node;
class Edge;

#ifdef HASH
#define INF 100
#define HASHSIZE 0x10000
#define CHUNKSIZE 0x100000
#else
#define INF 9999
#endif

typedef struct tagCoords
{
	union
	{
		__int32	v32;
		struct
		{
			__int8 x1;
			__int8 y1;
			__int8 x2;
			__int8 y2;
		};
	};
} Coords;

typedef struct tagEdgeSet
{
	union
	{
		unsigned __int64 v64;
		unsigned __int32 v32[2];
	}
} EdgeSet;

typedef struct tagWord32
{
	union
	{
		unsigned __int32 v32;
		unsigned __int32 v16[2];
	}
} Word32;

class HashNode
{
public:
	HashNode () {next = NULL;}

	EdgeSet position;
	HashNode *next;
	__int8 alpha;
	__int8 beta;
	__int8 val;
	__int8 valid;
};

typedef HashNode *PHashNode;

class Edge
{
public:
	Edge ();
	inline int Interesting (void);

	Coords GetCoords (void);

#ifdef HASH
	EdgeSet	edgeSet;
#endif

	int		length;
	Node	*node[2];
	Edge	**pself[2];

	Edge	*next;
	Edge	*prev;
	Edge	*parent;
	int		removed;
	int		checkRedundant;

	Node	*split;		// so that we can decompose chains
};

class Node
{
public:
	Node ();

	Edge *NextEdge (Edge *prev);
	Node *NextFrag (Edge *next);
	Node *NextNode (Edge *next);
	void DetachEdge (Edge *pedge);

	int		x, y;
	Edge	*edge[4];
	int		degree;
	Edge	chainEdge;	// for splicing out the node
	int		owner;		// for nodes with degree 0.. who got the square?
	int		ground;		// this is a ground node
};

#define SAVE(x) new UndoInfo((int *) &(x))
#define SET(x,v) SAVE(x), (x)=(v)
#define UNDO(x) UndoInfo::Undo(x), (x) = NULL

class UndoInfo
{
public:
	UndoInfo (int *_p) {v = *_p ; p = _p ; next = list ; list = this;}

	static UndoInfo *GetList (void) {UndoInfo *u = list; list = NULL; return u;}
	static void Undo (UndoInfo *undo);

	int			v;
	int			*p;
	UndoInfo	*next;

	static UndoInfo *list;
};

class Move
{
public:
	Move () {preMove = postMove = NULL; move = NULL; loop = 0;}

	UndoInfo	*preMove;
	UndoInfo	*postMove;
	Edge		*move;
	int			loop;		// loop move?
};

typedef enum {mt_short, mt_chain, mt_cycle} MoveType;

class SearchMove
{
public:
	SearchMove () {val = -INF;}

	Edge *move;
	int val;
	MoveType type;
};

#define MAX_SORTED	7

class DBGame
{
public:
	DBGame (int w, int h);
	~DBGame ();

	void Log (char *message, ...);

	void Connect (Edge &edge, Node &node1, Node &node2);

	void NewGame (int w, int h);
	int AddLine (unsigned s, unsigned t, Direction dir);
	int PlayerMove (Edge *edge);
	void SetEdgeRemoved (Edge *edge);
	int RemoveEdge (Edge *edge);
	void DoMove (Edge *edge);
	void DoLoopMove (Edge *edge, Edge *removedEdge);
	void UndoMove (Edge *edge);
	void UndoSplice (Node *node);
	void UndoLoopMove (Edge *edge);
	void MakeFrags (Edge *chain, Edge *split);
	void SpliceNode (Node *node);
	void Redo (void);

	void Undo (void);

	void TopDownRestoreEdges (Edge *edge);

	void MyMove (void);
	void DoFrag (int i);
	void DoubleCross (void);
	int Evaluate (int alpha, int beta, int depth, int score);
	int FastEval (void);
	inline Edge *GetChain (void);
	inline Edge *GetCycle (void);

#ifdef HASH
	int EvaluateNoHash (int alpha, int beta, int depth, int score);
#endif

	int		width;
	int		height;
	Edge	horiz[21][21];
	Edge	vert[21][21];
	Node	square[21][21];

	Edge	moves[3];
	Edge	chains[MAX_SORTED];		// from ground to ground
	Edge	cycles[MAX_SORTED];		// 
	Edge	strings[MAX_SORTED];	// still part of a connected graph
	Edge	loops[MAX_SORTED];		// almost a cycle

	Node	*frag[200];
	int		fraglen[200];
	int		fragopen[200];	// is there a dangling edge?
	int		numfrags;

	int		searchDepth;	// maximum search depth
	int		timeLimit;		// time limit per move in seconds
	int		gameLimit;		// time limit per game in seconds
	int		leaves;
	int		prunes;

	// for search output
	int		last_leaves;
	int		last_depth;
	int		last_beta;

	// debugging
	int		m_pause;
	CString	msg;
	void	Pause (char *message, ...);

	int		turn;		// number of moves so far
	int		turnOver;	// is the current turn over?

	Move	rgmoves[400];
	int		nummoves;
	Coords	rgEdgeRemoved[800];
	int		numEdgesRemoved;
	int		maxEdgesRemoved;

#ifdef HASH
	PHashNode	*hash;
	HashNode	*chunk;
	int			nextfree;

	EdgeSet	position;
	EdgeSet edgeBit;
#endif

	HWND	hWnd;
	int		thinking;
	int		stop;
};