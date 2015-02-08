//
// dbgame.cpp
//

//#include "StdAfx.h"
#include "dbgame-nohash.h"
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <cstdio>

#include <QTimer>
#include <KDebug>

//#pragma warning(disable:4390)

using namespace dabble_nohash;

UndoInfo *UndoInfo::list;

// FILE *logfile;

void DBGame::Log (char *message, ...)
{
	FILE *f = NULL;
	va_list args;

	if (leaves < 883)
		return;

	f = fopen("dabble.log", "a+");
	va_start(args, message);
	vfprintf(f, message, args);
	fclose(f);

	va_end(args);
}

Node::Node ()
{
	degree = 0;
	ground = 0;
	for (int i = 0 ; i < 4 ; i++)
		edge[i] = NULL;
	chainEdge.split = this;
}

Edge *Node::NextEdge (Edge *prev)
{
	for (int i = 0 ; i < 4 ; i++)
		prev = (Edge *) ((intptr_t) prev ^ (intptr_t) edge[i]);
	return prev;
}

Node *Node::NextFrag (Edge *next)
{
	int k;

	if (!next)
		return NULL;
	k = (next->node[0] == this);
	if (*next->pself[k] == next)
		return next->node[k];
	return NULL;
}

Node *Node::NextNode (Edge *next)
{
	if (!next)
		return NULL;
	return (Node *) ((intptr_t) next->node[0] ^ (intptr_t) next->node[1] ^ (intptr_t) this);
}

void Node::DetachEdge (Edge *pedge)
{
	int k = (this == pedge->node[1]);
	SET(pedge->node[k], NULL);
	SET(*pedge->pself[k], NULL);
}

Edge::Edge ()
{
	length = 1;
	node[0] = node[1] = NULL;
	pself[0] = pself[1] = NULL;
	parent = NULL;
	next = prev = this;
	removed = 0;
	checkRedundant = 0;
}

Coords Edge::GetCoords (void)
{
	Coords c;
	if (node[0])
	{
		c.x1 = node[0]->x;
		c.y1 = node[0]->y;
	}
	else
		c.x1 = c.y1 = -1;
	if (node[1])
	{
		c.x2 = node[1]->x;
		c.y2 = node[1]->y;
	}
	else
		c.x2 = c.y2 = -1;
	return c;
}

#define INSERT(a,b)\
	(a)->next = (b)->next;\
	(a)->prev = (b);\
	(b)->next->prev = (a);\
	(b)->next = (a);

#define REMOVE(a)\
	(a)->next->prev = (a)->prev;\
	(a)->prev->next = (a)->next;

#define REINSERT(a)\
	(a)->next->prev = (a);\
	(a)->prev->next = (a);

void DBGame::Connect (Edge &edge, Node &node1, Node &node2)
{
	int i, j;
	Node *node[2];
	static int count = 0;
	Edge *e;

	node[0] = &node1;
	node[1] = &node2;
	
	for (i = 0 ; i < 2 ; i++)
	{
		for (j = 0 ; node[i]->edge[j] != NULL ; j++);
		node[i]->edge[j] = &edge;
		node[i]->degree++;
		edge.node[i] = node[i];
		edge.pself[i] = &node[i]->edge[j];
	}

	j = rand() % (count + 1);
	e = &moves[0];
	for (i = 0 ; i < j ; i++)
		e = e->next;

	INSERT(&edge, e);
	count++;
}

DBGame::DBGame (int w, int h) : KSquaresAi(w - 1, h - 1)
{
	kDebug() << "width, height: " << width << " x " << height;
	int i, j;

	width = w;
	height = h;

// 	logfile = fopen("dabble.log", "w");
	long seed = time(NULL) & 255;
// 	fprintf(logfile, "Using random seed %d\n", seed);
// 	fprintf(logfile, "%d, %d\n", width, height);
// 	fclose(logfile);
	TRACE("Using random seed %d\n", seed);
	srand(seed);

	for (i = 0 ; i < w + 1 ; i++)
	{
		for (j = 0 ; j < h + 1 ; j++)
		{
			square[i][j].x = i;
			square[i][j].y = j;
		}
	}
	for (i = 0 ; i < w + 1 ; i++)
		square[i][0].ground = square[i][h].ground = 1;
	for (j = 0 ; j < h + 1 ; j++)
		square[0][j].ground = square[w][j].ground = 1;

	for (i = 1 ; i < w ; i++)
	{
		for (j = 0 ; j < h ; j++)
			Connect(horiz[i][j], square[i][j], square[i][j+1]);
	}

	for (j = 1 ; j < h ; j++)
	{
		for (i = 0 ; i < w ; i++)
			Connect(vert[i][j], square[i][j], square[i+1][j]);
	}

	for (i = 0 ; i < MAX_SORTED ; i++)
		chains[i].length = cycles[i].length = strings[i].length = loops[i].length = 999999;

	numfrags = 0;
	leaves = 0;
	last_depth = last_leaves = last_beta = 0;
	thinking = 0;

	searchDepth = 20;
	timeLimit = 5;
	gameLimit = -5;

	UndoInfo::list = NULL;
	nummoves = 0;

	numEdgesRemoved = maxEdgesRemoved = 0;
	turn = 0;
	turnOver = 1;
	
	dabbleTimer = QElapsedTimer();
	lastHistoryIndex = 0;
}

DBGame::~DBGame ()
{
	UNDO(UndoInfo::list);
	while (nummoves--)
	{
		UNDO(rgmoves[nummoves].postMove);
		UNDO(rgmoves[nummoves].preMove);
	}
}


Coords DBGame::indexToPoints(const int lineIndex)
{
	kDebug() << "width, height: " << width << " x " << height;
	Coords c;
  int index2 = lineIndex % ( ( 2 * (width-1) ) + 1 );
  c.y1 = lineIndex / ( ( 2 * (width-1) ) + 1) ;
  KSquares::Direction dir = aiFunctions::lineDirection(width-1, height-1, lineIndex);
  if (dir == KSquares::HORIZONTAL)
  {
    c.x1 = index2;
    c.y2 = c.y1;
    c.x2 = c.x1 + 1;
  }
  else 
  {
    c.x1 = index2 - (width-1);
    c.y2 = c.y1 + 1;
    c.x2 = c.x1;
  }
  //c.y1 = height - 1 - c.y1;
  //c.y2 = height - 1 - c.y2;
	
	return c;
}

int DBGame::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	kDebug() << "choosing line...";
	int line = -1;
	
	if (!dabbleTimer.isValid())
	{
		dabbleTimer.start();
	}
	else
	{
		dabbleTimer.restart();
	}
	//QTimer::singleShot(5000, this, SLOT(timeUp()));
	
	while (lastHistoryIndex < lineHistory.size())
	{
		Coords c = indexToPoints(lineHistory[lastHistoryIndex].line);
		rgEdgeRemoved[maxEdgesRemoved] = c;
		maxEdgesRemoved++;
		lastHistoryIndex++;
		if (maxEdgesRemoved != lastHistoryIndex)
		{
			TRACE("sth. went wrong");
		}
	}
	while (numEdgesRemoved < maxEdgesRemoved && !stop)
	{
		Redo();
		numEdgesRemoved++;
	}
	/*
	if (newLines.size() != previousLines.size())
	{
		previousLines.clear();
		previousLines.append(newLines);
		for (int i = 0; i < newLines.size(); i++)
		{
			Coords c = indexToPoints(i);
			rgEdgeRemoved[maxEdgesRemoved] = c;
			maxEdgesRemoved++;
		}
	}
	else
	{
		for (int i = 0; i < newLines.size(); i++)
		{
			if (newLines[i] && !previousLines[i])
			{
				Coords c = indexToPoints(i);
				rgEdgeRemoved[maxEdgesRemoved] = c;
				maxEdgesRemoved++;
			}
		}
	}
	*/
	MyMove();
	
	kDebug() << "rgmoves[nummoves-1].node[0] = (" << rgmoves[nummoves-1].move->node[0]->x << ", " << rgmoves[nummoves-1].move->node[0]->y << ") -- (" << rgmoves[nummoves-1].move->node[1]->x << ", " << rgmoves[nummoves-1].move->node[1]->y << ")";
	
	QPoint p1(rgmoves[nummoves-1].move->node[0]->x, rgmoves[nummoves-1].move->node[0]->y);
	QPoint p2(rgmoves[nummoves-1].move->node[1]->x, rgmoves[nummoves-1].move->node[1]->y);
	line = Board::pointsToIndex(p1, p2, width, height);
	kDebug() << "line index: " << line;
	
	if (newLines[line] || line < 0 || line >= newLines.size())
	{
		for (int i = 0; i < newLines.size(); i++)
		{
			if (!newLines[i])
				return i;
		}
	}
	
	kDebug() << "returned line: " << line;
	return line;
}

void DBGame::timeUp()
{
	kDebug() << "time is up!";
	TRACE("TIME UP SLOT");
	stop = 1;
	thinking = 0;
}

// returns TRUE if the player can keep moving, FALSE if it's the computer's turn
int DBGame::AddLine (unsigned s, unsigned t, Direction dir)
{
	if (dir == dir_horizontal)
	{
		if (!horiz[s][t].removed)
			return PlayerMove(&horiz[s][t]);
	}
	else if (!vert[s][t].removed)
		return PlayerMove(&vert[s][t]);
	return 1;
}

// returns TRUE if the player can keep moving, FALSE if it's the computer's turn
int DBGame::PlayerMove (Edge *edge)
{
	if (turnOver)
	{
		SAVE(turn);
		turn++;
	}
	SAVE(turnOver);
	if (turnOver = !RemoveEdge(edge))
	{
		rgmoves[nummoves++].postMove = UndoInfo::GetList();
		return 0;
	}
	return 1;
}

void DBGame::SetEdgeRemoved (Edge *edge)
{
	Coords c = edge->GetCoords();

// 	logfile = fopen("dabble.log", "a+");
// 	fprintf(logfile, "%d: (%d, %d) - (%d, %d)\n", nummoves, c.x1, c.y1, c.x2, c.y2);
// 	fclose(logfile);

	SET(edge->removed, turn);
	SAVE(numEdgesRemoved);
	if (numEdgesRemoved >= maxEdgesRemoved || c.v32 != rgEdgeRemoved[numEdgesRemoved].v32)
	{
		maxEdgesRemoved = numEdgesRemoved + 1;
		rgEdgeRemoved[numEdgesRemoved] = c;
	}
	numEdgesRemoved++;
}

void DBGame::Redo (void)
{
	int i, j;
	Edge *edge = NULL;
	Coords c = rgEdgeRemoved[numEdgesRemoved];

	for (i = 1 ; i < width ; i++)
	{
		for (j = 0 ; j < height ; j++)
		{
			if (horiz[i][j].GetCoords().v32 == c.v32)
				edge = &horiz[i][j];
		}
	}

	for (j = 1 ; j < height ; j++)
	{
		for (i = 0 ; i < width ; i++)
		{
			if (vert[i][j].GetCoords().v32 == c.v32)
				edge = &vert[i][j];
		}
	}

	if (edge)
		PlayerMove(edge);
}

int DBGame::RemoveEdge (Edge *edge)
{
	int i, goagain, len;
	Edge *pedge;
	Node *pfrag, *nextfrag;

	SetEdgeRemoved(edge);

	if (!edge->parent)
	{
		rgmoves[nummoves].preMove = UndoInfo::GetList();
		rgmoves[nummoves].move = edge;
		DoMove(edge);
		return 0;
	}
	
	for (i = 0 ; i < numfrags ; i++)
	{
		pfrag = frag[i];
		len = 1;
		pedge = NULL;
		while (pfrag)
		{
			pedge = pfrag->NextEdge(pedge);
			nextfrag = pfrag->NextFrag(pedge);
			if (pedge == edge)
			{
				goagain = 0;
				if (nextfrag)
				{
					SAVE(nextfrag->degree);
					if (--nextfrag->degree)
					{
						SET(frag[numfrags], nextfrag);
						SET(fraglen[numfrags], fraglen[i] - len);
						SET(fragopen[numfrags], fragopen[i]);
						SET(fraglen[i], len);
						nextfrag->DetachEdge(pedge);
						SAVE(numfrags);
						numfrags++;
					}
					else
					{
						nextfrag->owner = nummoves & 1;
						goagain = 1;
					}
				}
				pfrag->DetachEdge(pedge);
				SAVE(pfrag->degree);
				pfrag->degree--;
				SET(fragopen[i], 0);
				if (!pfrag->degree)
				{
					goagain = 1;
					pfrag->owner = nummoves & 1;
					SAVE(numfrags);
					numfrags--;
					SET(frag[i], frag[numfrags]);
					SET(fraglen[i], fraglen[numfrags]);
					SET(fragopen[i], fraglen[numfrags]);
				}
				return goagain;
			}
			pfrag = nextfrag;
			len++;
		}
	}

	rgmoves[nummoves].preMove = UndoInfo::GetList();
	for (pedge = edge ; pedge->parent ; pedge = pedge->parent);
	if (pedge->node[0] && pedge->node[0] == pedge->node[1] && pedge->node[0]->degree == 3)
	{
		DoLoopMove(pedge, edge);
		pedge = pedge->parent;
		rgmoves[nummoves].loop = 1;
	}
	else
	{
		rgmoves[nummoves].loop = 0;
		DoMove(pedge);
	}
	rgmoves[nummoves].move = pedge;
	MakeFrags(pedge, edge);
	return 0;
}

void DBGame::MakeFrags (Edge *chain, Edge *split)
{
	int i, len;
	Node *pnode;
	Edge *pedge;

	TopDownRestoreEdges(chain);
	for (i = 0 ; i < 2 ; i++)
	{
		if (split->node[i] != chain->node[0] && split->node[i] != chain->node[1])
		{
			SET(frag[numfrags], split->node[i]);
			len = 0;
			pnode = split->node[i];
			pedge = split;
			do
			{
				pedge = pnode->NextEdge(pedge);
				pnode = pnode->NextFrag(pedge);
				len++;
			} while (pnode && pnode != split->node[i]);
			SET(fragopen[numfrags], (pedge != NULL) && (pedge != split));
			SET(fraglen[numfrags], len);
			SAVE(numfrags);
			numfrags++;
			SAVE(split->node[i]->degree);
			split->node[i]->degree--;
			SET(*split->pself[i], NULL);
		}
	}
	if (chain->node[0] == chain->node[1] && fraglen[numfrags-1] == fraglen[numfrags-2])
	{
		SAVE(numfrags);
		numfrags--;
	}
}

void DBGame::TopDownRestoreEdges (Edge *edge)
{
	int j;
	Node *node;

	if (edge->length == 1)
	{
		if (*edge->pself[0])
			SET(*edge->pself[0], edge);
		if (*edge->pself[1])
			SET(*edge->pself[1], edge);
		return;
	}
	node = edge->split;
	for (j = 0 ; j < 4 ; j++)
	{
		if (node->edge[j])
			TopDownRestoreEdges(node->edge[j]);
	}
}

void DBGame::DoLoopMove (Edge *edge, Edge *removedEdge)
{
	Node *node;
	Edge *edges[2], *newedge;
	int j, k;
	
	// detatch and splice
	node = edge->node[0];
	node->degree = 2;
	if (edge->pself[0] == removedEdge->pself[0] || edge->pself[0] == removedEdge->pself[1])
		*edge->pself[0] = NULL;
	else
		*edge->pself[1] = NULL;
	edges[0] = edge;
	edges[1] = node->NextEdge(edge);
	newedge = &node->chainEdge;
	
	// form a new chain from two edges
	newedge->length = edges[0]->length + edges[1]->length;
	for (j = 0 ; j < 2 ; j++)
	{
		// remove the edge and indicate that it has a parent edge
		edges[j]->parent = newedge;
		REMOVE(edges[j]);
		
	}
	// don't replace the old edge with the new edge at end 0 - just update the node pointer
	newedge->node[0] = node;
	newedge->pself[0] = NULL;

	// replace the old edge with the new edge at end 1
	k = (edges[1]->node[0] == node);
	newedge->node[1] = edges[1]->node[k];
	newedge->pself[1] = edges[1]->pself[k];

	edge = newedge;

	// remove the new edge - only need to deal with side 1
	node = edge->node[1];
	node->degree--;
	*edge->pself[1] = NULL;
	if (node->degree == 2)
		SpliceNode(node);
}

// called with *parent* of loop edge, which is created in DoLoopMove
void DBGame::UndoLoopMove (Edge *edge)
{
	Node *node;
	Edge *edges[3];
	int j, k;

	node = edge->node[1];
	node->degree++;
	if (node->degree == 3)
	{
		// undo splice
		REMOVE(&node->chainEdge);
		for (j = k = 0 ; j < 4 ; j++)
		{
			edges[k] = node->edge[j];
			k += (node->edge[j] != NULL);
		}
		REINSERT(edges[1]);
		edges[1]->parent = NULL;
		
		if (edges[1] != edges[0])
		{
			*edges[1]->pself[0] = *edges[1]->pself[1] = edges[1];
			REINSERT(edges[0]);
			edges[0]->parent = NULL;
			*edges[0]->pself[0] = *edges[0]->pself[1] = edges[0];
		}
	}

	node = edge->split;
	for (j = k = 0 ; j < 4 ; j++)
	{
		edges[k] = node->edge[j];
		k += (node->edge[j] != NULL);
	}
	// edges[0] must be the loop
	if (edges[0]->node[0] != edges[0]->node[1])
	{
		edges[2] = edges[0];
		edges[0] = edges[1];
		edges[1] = edges[2];
	}

	*edges[0]->pself[0] = *edges[0]->pself[1] = edges[0];
	*edges[1]->pself[0] = *edges[1]->pself[1] = edges[1];
	edges[0]->parent = edges[1]->parent = NULL;
	edges[0]->node[0]->degree = 3;
	REINSERT(edges[1]);
	REINSERT(edges[0]);
}

void DBGame::SpliceNode (Node *node)
{
	int j, k;
	Edge *pedge, *newedge, *edges[3];

	for (j = k = 0 ; j < 4 ; j++)
	{
		if (edges[k] = node->edge[j])
			k++;
	}
	newedge = &node->chainEdge;

	// check for a cycle
	if (edges[0] == edges[1])
	{
		newedge->length = edges[0]->length;
		
		// remove the edge and indicate that it has a parent edge
		edges[0]->parent = newedge;
		REMOVE(edges[0]);
		
		newedge->node[0] = newedge->node[1] = NULL;
		
		// put the new edge in the list of cycles
		k = edges[0]->length;
		if (k < MAX_SORTED - 1)
		{
			INSERT(newedge, &cycles[k]);
		}
		else
		{
			for (pedge = &cycles[MAX_SORTED - 1] ; edges[0]->length > pedge->next->length ; pedge = pedge->next);
			INSERT(newedge, pedge);
		}
	}
	else
	{
		// form a new chain from two edges
		newedge->length = edges[0]->length + edges[1]->length;
		for (j = 0 ; j < 2 ; j++)
		{
			// remove the edge and indicate that it has a parent edge
			edges[j]->parent = newedge;
			REMOVE(edges[j]);
			
			// replace the old edge with the new edge at the other end
			k = (edges[j]->node[0] == node);
			newedge->node[j] = edges[j]->node[k];
			newedge->pself[j] = edges[j]->pself[k];
			*newedge->pself[j] = newedge;
		}
		
		// put the edge in a list somewhere
		k = newedge->length - 1;
		if (k < 3)
		{
			INSERT(newedge, &moves[k]);
		}
		else
		{
			Edge *rgedge;
			if (newedge->node[0] == newedge->node[1])
				rgedge = loops;
			else if (newedge->node[0]->ground && newedge->node[1]->ground)
				rgedge = chains;
			else
				rgedge = strings;
			if (k < MAX_SORTED - 1)
			{
				INSERT(newedge, &rgedge[k]);
			}
			else
			{
				for (pedge = &rgedge[MAX_SORTED - 1] ; newedge->length > pedge->next->length ; pedge = pedge->next);
				INSERT(newedge, pedge);
			}
		}
	}
}

void DBGame::UndoSplice (Node *node)
{
	Edge *edges[3];
	int j, k;
	
	REMOVE(&node->chainEdge);
	for (j = k = 0 ; j < 4 ; j++)
	{
		if (edges[k] = node->edge[j])
			k++;
	}
	//Log("k = %d\n", k);
	REINSERT(edges[1]);
	edges[1]->parent = NULL;
	
	if (edges[1] != edges[0])
	{
		*edges[1]->pself[0] = *edges[1]->pself[1] = edges[1];
		REINSERT(edges[0]);
		edges[0]->parent = NULL;
		*edges[0]->pself[0] = *edges[0]->pself[1] = edges[0];
	}
}

void DBGame::DoMove (Edge *edge)
{
	Node *node;

	/*
	if (edge->node[0])
		TRACE("Domove:  (%d, %d) - (%d, %d)  [%d]\n", edge->node[0]->x, edge->node[0]->y, edge->node[1]->x, edge->node[1]->y, edge->length);
	else
		TRACE("Domove:  cycle (%d, %d)  [%d]\n", edge->split->x, edge->split->y, edge->length);
	*/

	// remove the move
	REMOVE(edge);

	// deal with each side
	node = edge->node[0];
	if (node == edge->node[1])
	{
		if (node)
		{
			// node *must* be of degree 4!!
			assert(node->degree == 4);

			node->degree = 2;
			*edge->pself[0] = *edge->pself[1] = NULL;
			SpliceNode(node);
		}
	}
	else
	{
		if (node)
		{
			node->degree--;
			*edge->pself[0] = NULL;
			if (node->degree == 2)
				SpliceNode(node);
		}
		node = edge->node[1];
		if (node)
		{
			node->degree--;
			*edge->pself[1] = NULL;
			if (node->degree == 2)
				SpliceNode(node);
		}
	}
}

void DBGame::UndoMove (Edge *edge)
{
	Node *node;

	node = edge->node[1];
	if (node == edge->node[0])
	{
		if (node)
		{
			// node *must* be of degree 2!!
			assert(node->degree == 2);

			node->degree = 4;
			UndoSplice(node);
			*edge->pself[0] = *edge->pself[1] = edge;
		}
	}
	else
	{
		if (node)
		{
			node->degree++;
			if (node->degree == 3)
				UndoSplice(node);
			*edge->pself[1] = edge;
		}
		node = edge->node[0];
		if (node)
		{
			node->degree++;
			if (node->degree == 3)
				UndoSplice(node);
			*edge->pself[0] = edge;
		}
	}
	REINSERT(edge);
}

void DBGame::Undo (void)
{
	if (UndoInfo::list)
	{
		UNDO(UndoInfo::list);
	}
	else if (nummoves)
	{
		--nummoves;
		UNDO(rgmoves[nummoves].postMove);
		if (rgmoves[nummoves].move)
		{
			if (rgmoves[nummoves].loop)
				UndoLoopMove(rgmoves[nummoves].move);
			else
				UndoMove(rgmoves[nummoves].move);
		}
		UNDO(rgmoves[nummoves].preMove);
	}
}

void UndoInfo::Undo (UndoInfo *undo)
{
	while (undo)
	{
		UndoInfo *next = undo->next;
		*undo->p = undo->v;
		delete undo;
		undo = next;
	}
}

void DBGame::DoFrag (int i)
{
	Node *node = frag[i];
	Edge *edge = NULL;

	while (node)
	{
		SET(node->degree, 0);
		node->owner = nummoves & 1;
		edge = node->NextEdge(edge);
		if (edge)
			SetEdgeRemoved(edge);
		node = node->NextFrag(edge);
	}
	SAVE(numfrags);
	numfrags--;
	SET(frag[i], frag[numfrags]);
	SET(fraglen[i], fraglen[numfrags]);
	SET(fragopen[i], fragopen[numfrags]);
}

void DBGame::DoubleCross (void)
{
	Node *dcn, *node = frag[0];
	Edge *dce, *edge = NULL;
	int steps = fragopen[0] ? fraglen[0] - 2 : fraglen[0] - 4;

	for (int i = 0 ; i < steps ; i++)
	{
		SET(node->degree, 0);
		node->owner = nummoves & 1;
		edge = node->NextEdge(edge);
		node = node->NextFrag(edge);
		SetEdgeRemoved(edge);
	}
	dcn = node;
	dce = edge;
	edge = node->NextEdge(edge);
	node = node->NextFrag(edge);
	edge = node->NextEdge(edge);

	if (dce)
	{
		SET(frag[0], dcn);
		SET(fraglen[0], fragopen[0] ? 2 : 4);
		dcn->DetachEdge(dce);
		SAVE(dcn->degree);
		dcn->degree--;
	}

	RemoveEdge(edge);
}

inline int Edge::Interesting (void)
{
	if (!node[0]->ground)
	{
		if (node[0]->edge[0] && node[0]->edge[0] != this && node[0]->edge[0]->length < 4)
			return 1;
		if (node[0]->edge[1] && node[0]->edge[1] != this && node[0]->edge[1]->length < 4)
			return 1;
		if (node[0]->edge[2] && node[0]->edge[2] != this && node[0]->edge[2]->length < 4)
			return 1;
		if (node[0]->edge[3] && node[0]->edge[3] != this && node[0]->edge[3]->length < 4)
			return 1;
	}
	if (!node[1]->ground)
	{
		if (node[1]->edge[0] && node[1]->edge[0] != this && node[1]->edge[0]->length < 4)
			return 1;
		if (node[1]->edge[1] && node[1]->edge[1] != this && node[1]->edge[1]->length < 4)
			return 1;
		if (node[1]->edge[2] && node[1]->edge[2] != this && node[1]->edge[2]->length < 4)
			return 1;
		if (node[1]->edge[3] && node[1]->edge[3] != this && node[1]->edge[3]->length < 4)
			return 1;
	}

	return 0;
}

inline Edge *DBGame::GetChain (void)
{
	for (int i = 3 ; i < MAX_SORTED ; i++)
	{
		if (chains[i].next != &chains[i])
			return chains[i].next;
	}
	return NULL;
}

inline Edge *DBGame::GetCycle (void)
{
	for (int i = 4 ; i < MAX_SORTED ; i += 2)
	{
		if (cycles[i].next != &cycles[i])
			return cycles[i].next;
	}
	return NULL;
}

void DBGame::MyMove (void)
{
	int i, j, k;
	Edge *move, *edge, *bestMove, *edge2;
	int beta, depth;
	int val, prevleaves;
	int	uninteresting;	// have we done an uninteresting move?
	Node *node, *node2;
	int chainlen;

	SAVE(turn);
	turn++;
	turnOver = 1;
	bestMove = NULL;
	prevleaves = 0;

	for (i = 0 ; i < numfrags ; i++)
	{
		if (fraglen[i] < 2 || (!fragopen[i] && fraglen[i] < 4))
			DoFrag(i--);
	}
	while (numfrags > 1)
		DoFrag(fragopen[0]);

	// set up the moves
	int cmoves = 0, imove = 0;
	SearchMove *searchMoves;

	uninteresting = 0;
	for (i = 0 ; i < 3 ; i++)
	{
		for (edge = moves[i].next ; edge != &moves[i] ; edge = edge->next)
		{
			int f = edge->Interesting();
			if (f || !uninteresting)
			{
				cmoves++;
				if (!f)
					uninteresting = 1;
			}	
		}
	}
	if (cmoves)
	{
		searchMoves = new SearchMove[cmoves];
		uninteresting = 0;
		for (i = 0 ; i < 3 ; i++)
		{
			for (edge = moves[i].next ; edge != &moves[i] ; edge = edge->next)
			{
				int f = edge->Interesting();
				if (f || !uninteresting)
				{
					searchMoves[imove].move = edge;
					searchMoves[imove++].type = mt_short;
					if (!f)
						uninteresting = 1;
				}	
			}
		}
	}
	else
	{
		chainlen = 9999;
		if (edge = GetChain())
		{
			chainlen = edge->length;
			cmoves++;
		}
		if (edge = GetCycle())
			cmoves++;
		for (i = 3 ; i < MAX_SORTED ; i++)
		{
			for (edge = strings[i].next ; edge != &strings[i] ; edge = edge->next)
			{
				if (edge->length < chainlen)
					cmoves++;
			}
			for (edge = loops[i].next ; edge != &loops[i] ; edge = edge->next)
			{
				if (edge->length < chainlen && edge->node[0]->degree == 4)
					cmoves++;
			}
		}
		searchMoves = new SearchMove[cmoves];
		if (edge = GetChain())
		{
			searchMoves[imove].move = edge;
			searchMoves[imove++].type = mt_chain;
		}
		if (edge = GetCycle())
		{
			searchMoves[imove].move = edge;
			searchMoves[imove++].type = mt_cycle;
		}
		for (i = 3 ; i < MAX_SORTED ; i++)
		{
			for (edge = strings[i].next ; edge != &strings[i] ; edge = edge->next)
			{
				if (edge->length < chainlen)
				{
					searchMoves[imove].move = edge;
					searchMoves[imove++].type = mt_chain;
				}
			}
			for (edge = loops[i].next ; edge != &loops[i] ; edge = edge->next)
			{
				if (edge->length < chainlen && edge->node[0]->degree == 4)
				{
					searchMoves[imove].move = edge;
					searchMoves[imove++].type = mt_chain;
				}
			}
		}
	}

	// check for multiple edges from a single node to the ground or from the ground to the ground
	for (i = 0 ; i < cmoves ; i++)
	{
		edge = searchMoves[i].move;
		if (edge->node[0] && (edge->node[0]->ground || edge->node[1]->ground))
		{
			for (j = i+1 ; j < cmoves ; j++)
			{
				edge2 = searchMoves[j].move;
				if (edge2->node[0] && (edge2->node[0]->ground || edge2->node[1]->ground))
				{
					node = edge->node[0]->ground ? edge->node[1] : edge->node[0];
					node2 = edge2->node[0]->ground ? edge2->node[1] : edge2->node[0];
					if ((node->ground && node2->ground) || (node == node2 && edge->length == edge2->length))
					{
						edge->checkRedundant = 1;
						edge2->checkRedundant = 1;
						if (edge2->length < edge->length)
						{
							searchMoves[i] = searchMoves[j];
							edge = edge2;
						}
						for (k = j ; k < cmoves - 1 ; k++)
							searchMoves[k] = searchMoves[k+1];
						cmoves--;
						j--;
					}
				}
			}
		}
	}

	for (depth = 1 ; depth <= searchDepth && ((!stop && !dabbleTimer.hasExpired(5000)) || depth == 1) ; depth++)
	{
		move = NULL;
		leaves = 0;
		beta = -INF;

		// Sort the moves
		SearchMove sm;
		for (i = 0 ; i < cmoves - 1 ; i++)
		{
			for (j = cmoves - 1 ; j > i ; j--)
			{
				if (searchMoves[j].val > searchMoves[j-1].val)
				{
					sm = searchMoves[j];
					searchMoves[j] = searchMoves[j-1];
					searchMoves[j-1] = sm;
				}
			}
		}

		// Find the best move!
		for (imove = 0 ; imove < cmoves ; imove++)
		{
			edge = searchMoves[imove].move;
			DoMove(edge);
			if (searchMoves[imove].type == mt_short)
				val = Evaluate(-beta, -INF, depth, 1 - edge->length);
			else
			{
				val = Evaluate(INF, -INF, depth, 0);
				if (searchMoves[imove].type == mt_chain)
				{
					if (val > 2)
						val = 5 - edge->length - val;
					else
						val -= edge->length - 1;
				}
				else
				{
					if (val > 4)
						val = 8 - edge->length - val;
					else
						val -= edge->length;
				}
			}
			searchMoves[imove].val = val;
			if ((val > beta || (move != NULL && val == beta && edge->length < move->length)) && (!stop && !dabbleTimer.hasExpired(5000)))
			if (val > beta && (!stop && !dabbleTimer.hasExpired(5000)))
			{
				move = edge;
				beta = val;
			}
			UndoMove(edge);
		}

		if ((!stop && !dabbleTimer.hasExpired(5000)))
		{
			last_leaves = leaves;
			last_depth = depth;
			last_beta = beta;

			if (numfrags)
			{
				if (beta < -4 || (fragopen[0] && beta < -2))
				{
					if (fragopen[0])
						last_beta = fraglen[0] - 4 - beta;
					else
						last_beta = fraglen[0] - 8 - beta;
				}
				else
					last_beta += fraglen[0];
			}

			TRACE("Searched %d leaves at depth %d (beta = %d)\n", leaves, depth, last_beta);

			//::PostMessage(hWnd, WM_CHAR, 18, 0);

			if (leaves == prevleaves && move == bestMove)
			{
				last_depth--;
				break;
			}
			prevleaves = leaves;
			bestMove = move;
		}
		else
		{
			TRACE("Time's up!\n");

			// commented out until I can fix the flip-flopping problem
			/*
			if (beta > last_beta)
			{
				bestMove = move;
				last_depth = depth;
				last_leaves = leaves;
				last_beta = beta;
				TRACE("Using partial search result (%d > %d)\n", beta, last_beta);
			}
			*/
		}
	}

	for (i = 0 ; i < imove ; i++)
		searchMoves[i].move->checkRedundant = 0;
	delete[] searchMoves;

	if (!bestMove)
		bestMove = move;
	rgmoves[nummoves].loop = 0;
	move = bestMove;

	if (!move)
	{
		if (numfrags)
		{
			DoFrag(0);
			rgmoves[nummoves++].postMove = UndoInfo::GetList();
		}
		last_beta = 0;
		return;
	}

	if (numfrags)
	{
		if (beta < -4 || (fragopen[0] && beta < -2))
		{
			DoubleCross();
			rgmoves[nummoves++].postMove = UndoInfo::GetList();
			return;
		}
		else
			DoFrag(0);
	}

	switch (move->length)
	{
	case 1:
		RemoveEdge(move);
		break;	

	case 2:
		for (i = 0 ; !move->split->edge[i] ; i++);
		RemoveEdge(move->split->edge[i]);
		break;

	case 3:
		node = move->split;
		for (i = 0 ; !(node->edge[i] && node->edge[i]->length == 2) ; i++);
		edge = node->edge[i];
		node2 = edge->split;
		for (i = 0 ; !(node2->edge[i] && node2->NextNode(node2->edge[i]) == node) ; i++);
		RemoveEdge(node2->edge[i]);
		break;

	default:
		while (move->length > 1)
		{
			node = move->split;
			for (i = 0 ; !node->edge[i] ; i++);
			move = node->edge[i];
		}
		RemoveEdge(move);
		break;
	}
	rgmoves[nummoves++].postMove = UndoInfo::GetList();
}

typedef struct tagWord16
{
	union
	{
		__int16	v16;
		__int8	v8[2];
	};
} Word16;

#define CHECKVAL if (val > beta) {if (val >= alpha) return -val; beta = val;}

int DBGame::Evaluate (int alpha, int beta, int depth, int score)
{
	int i;
	Edge *edge;
	int val;
	Word16 interesting;	// have we done an uninteresting (v8[0]) or interesting (v8[1]) move?
	int groundground = 0;
	int skip;
	int chainlen;

	//alpha = INF;
	//beta = -INF;

	if (stop)
		return -beta;

	//TRACE("Evaluate alpha %d beta %d depth %d score %d\n", alpha, beta, depth, score);

	if (!--depth)
	{
		val = FastEval();
		return val + score;

		//return FastEval() + score;
	}

	// First check the actual moves
	interesting.v16 = 0;
	for (i = 0 ; i < 3 ; i++)
	{
		for (edge = moves[i].next ; edge != &moves[i] ; edge = edge->next)
		{
			int f = edge->Interesting();
			if (f || !interesting.v8[0])
			{
				if (edge->checkRedundant)
				{
					if (edge->node[0]->ground && edge->node[1]->ground)
					{
						if (groundground)
							continue;
						groundground = 1;
					}
					else
					{
						skip = 0;
						Node *node = edge->node[edge->node[0]->ground];
						for (Edge *edge2 = edge->next ; edge2 != &moves[i] ; edge2 = edge2->next)
						{
							if ((edge2->node[0]->ground ^ edge2->node[1]->ground) && (edge2->node[edge2->node[0]->ground] == node))
							{
								skip = 1;
								break;
							}
						}
						if (skip)
							continue;
					}
				}

				// do the move
				DoMove(edge);
				val = Evaluate(-beta, -alpha, depth, -score - i);
				UndoMove(edge);
				interesting.v8[f] = 1;

				CHECKVAL;
			}
		}
	}

	// If there weren't any, then check exactly one cycle, one chain, and all strings
	if (!interesting.v16)
	{
		chainlen = 9999;
		if (edge = GetChain())
		{
			chainlen = edge->length;
			DoMove(edge);

			val = Evaluate(INF, -INF, depth, 0);
			if (val > 2)
				val = 5 - edge->length - val;
			else
				val -= edge->length - 1;
			val -= score;

			UndoMove(edge);
			CHECKVAL;

			interesting.v16 = 1;
		}
		if (edge = GetCycle())
		{
			DoMove(edge);

			val = Evaluate(INF, -INF, depth, 0);
			if (val > 4)
				val = 8 - edge->length - val;
			else
				val -= edge->length;
			val -= score;

			UndoMove(edge);
			CHECKVAL;

			interesting.v16 = 1;
		}
		for (i = 3 ; i < MAX_SORTED ; i++)
		{
			for (edge = strings[i].next ; edge != &strings[i] ; edge = edge->next)
			{
				if (edge->length >= chainlen)
					break;
				DoMove(edge);

				val = Evaluate(INF, -INF, depth, 0);
				if (val > 2)
					val = 5 - edge->length - val;
				else
					val -= edge->length - 1;
				val -= score;

				UndoMove(edge);
				CHECKVAL;

				interesting.v16 = 1;
			}
			for (edge = loops[i].next ; edge != &loops[i] ; edge = edge->next)
			{
				if (edge->length >= chainlen || edge->node[0]->degree != 4)
					break;
				DoMove(edge);

				val = Evaluate(INF, -INF, depth, 0);
				if (val > 2)
					val = 5 - edge->length - val;
				else
					val -= edge->length - 1;
				val -= score;

				UndoMove(edge);
				CHECKVAL;

				interesting.v16 = 1;
			}
		}
	}
	if (interesting.v16)
		return -beta;
	else
	{
		leaves++;
		return score;
	}
}

inline int fast_rand (void)
{
	static union
	{
		__int32 v32;
		__int16 v16[2];
	} r;

	return r.v32 += 1 + r.v16[1];
}

int DBGame::FastEval (void)
{
	int i, j;
	Edge *edge;
	int val;
	int doublecross = 0;

	// First check the actual moves
	for (i = 0 ; i < 3 ; i++)
	{
		edge = moves[i].next;
		if (edge != &moves[i])
		{
			DoMove(edge);
			val = FastEval() - i;
			UndoMove(edge);
			return -val;
		}
	}

	// If there weren't any, then check for a string, chain or cycle
	for (i = 3 ; i < MAX_SORTED ; i++)
	{
		edge = strings[i].next;
		if (edge != &strings[i])
		{
			//Log("(%d, %d) (%d, %d) %d %d\n", edge->node[0]->x, edge->node[0]->y, edge->node[1]->x, edge->node[1]->y, edge->node[0]->degree, edge->node[1]->degree);
			DoMove(edge);
			val = FastEval();
			UndoMove(edge);
			if (val > 2)
				val = 5 - edge->length - val;
			else
				val -= edge->length - 1;
			return -val;
		}
		
		edge = chains[i].next;
		if (edge != &chains[i])
		{
			DoMove(edge);
			val = FastEval();
			UndoMove(edge);
			if (val > 2)
				val = 5 - edge->length - val;
			else
				val -= edge->length - 1;
			return -val;
		}

		for (edge = loops[i].next ; edge != &loops[i] ; edge = edge->next)
		{
			if (edge->node[0]->degree == 4)
			{
				DoMove(edge);
				val = FastEval();
				UndoMove(edge);
				if (val > 2)
					val = 5 - edge->length - val;
				else
					val -= edge->length - 1;
				return -val;
			}
		}
		
		edge = cycles[i].next;
		if (edge != &cycles[i])
		{
			DoMove(edge);
			val = FastEval();
			UndoMove(edge);
			if (val > 4)
				val = 8 - edge->length - val;
			else
				val -= edge->length;
			return -val;
		}
	}
	leaves++;
	return 0;
}

/*
void DBGame::Pause (char *message, ...)
{
	va_list args;
	va_start(args, message);
	msg.FormatV(message, args);
	va_end(args);
	m_pause = 1;

	::SendMessage(hWnd, WM_CHAR, 18, 0);
	while (m_pause)
		Sleep(100);
}
*/

#include "dbgame-nohash.moc"
