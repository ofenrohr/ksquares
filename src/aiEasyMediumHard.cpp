/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiEasyMediumHard.h"

#include <KDebug>

aiEasyMediumHard::aiEasyMediumHard(int newPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
}

QString aiEasyMediumHard::getName()
{
	switch (level)
	{
		case 0: return "easy";
		case 1: return "medium";
		case 2: return "hard";
		default: return "aiEasyMediumHard";
	}
}

int aiEasyMediumHard::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners)
{
	if (newLines.size() != linesSize)
	{
		kFatal() << "something went terribly wrong: newLines.size() != linesSize";
	}
	// put lines into local board representation
	for (int i = 0; i < linesSize; ++i) lines[i] = newLines[i];
	// remember square owner table (might not be necessary)
	//squareOwners = newSquareOwners;
	// do the ai stuff:
	kDebug() << "incoming board:" << boardToString(lines, linesSize, width, height);
	QList<int> choiceList = findLinesCompletingBoxes(linesSize, lines);
  kDebug() << "finLinesCompletingBoxes returned: " << choiceList;
	if(choiceList.size() != 0)
	{
		if(level >= 2) // to play good ai has to look into the future game
		{
			QList<int> openLines; // list of not yet drawn lines
			openLines = aiFunctions::getFreeLines(lines, linesSize);
			kDebug() << "choosing from all possible lines";
			QList<int> choices=chooseLeastDamaging(openLines); // run extended damage control
			if(choices.size() > 0)
			{
				kDebug() << "AI: 4. Drawing line at index:" << choices.at(0);
				return choices.at(0);
			}
		}
		float randomFloat = ((float) rand()/(RAND_MAX + 1.0))*(choiceList.size()-1);
		int randChoice = (short)(randomFloat)/1;
		kDebug() << "AI: 1. Drawing line at index:" << choiceList.at(randChoice);
		return choiceList.at(randChoice);
	}
	
	choiceList = safeMoves(linesSize, lines);
  kDebug() << "safeMoves:" << linelistToString(choiceList);
	
	if(choiceList.size() != 0)
	{
		float randomFloat = ((float) rand()/(RAND_MAX + 1.0))*(choiceList.size()-1);
		int randChoice = (short)(randomFloat)/1;
		kDebug() << "choiceList: " << choiceList;
		kDebug() << "AI: 2. Drawing line at index:" << choiceList.at(randChoice);
		return choiceList.at(randChoice);
	}
	
	choiceList.clear();
	for(int i=0; i<linesSize; i++)	//have to take what's left
	{
		if(!lines[i])
		{
			QList<int> adjacentSquares = squaresFromLine(i);
			for(int j=0; j<adjacentSquares.size(); j++)
			{
				if(countBorderLines(adjacentSquares.at(j), lines) == 2 && !choiceList.contains(i))	//if 2 lines (they're all that's left!)
				{
					choiceList.append(i);
					//kDebug() << "AI: 3. Adding" << i << "to choices";
				}
			}
		}
	}
	if(level >= 1) //Hard(2/3)	//do some damage control :)
	{
		QList<int> goodChoiceList = chooseLeastDamaging(choiceList);
    kDebug() << "goodChoiceList: " << linelistToString(goodChoiceList);
		if(goodChoiceList.size() != 0)
		{
			float randomFloat = ((float) rand()/(RAND_MAX + 1.0))*(goodChoiceList.size()-1);
			int randChoice = (short)(randomFloat)/1;
			kDebug() << "AI: 3. Drawing line at index:" << goodChoiceList.at(randChoice);
			return goodChoiceList.at(randChoice);
		}
	}

	if(choiceList.size() != 0)
	{
		float randomFloat = ((float) rand()/(RAND_MAX + 1.0))*(choiceList.size()-1);
		int randChoice = (short)(randomFloat)/1;
		kDebug() << "AI: 3. Drawing line at index:" << choiceList.at(randChoice);
		return choiceList.at(randChoice);
	}
        return 0;
}

QList<int> aiEasyMediumHard::chooseLeastDamaging(const QList<int> &choiceList) const
{
	kDebug() << "AI: Checking" << choiceList.size() << "possible moves";
	kDebug() << "choiceList: " << linelistToString(choiceList, linesSize, width, height);
	QMap<int,int> linePointDamage;	//this will be a list of how damaging a certain move will be. Key = damage of move, Value = index of line
	QScopedArrayPointer<bool> linesCopy(new bool[linesSize]); //make temporary local copies of lists
	
	QMap<int, QSet<int> > chains; // this is a raw list of chains (which are sets of lines). key = random element of chain
	QMap<int, QSet<int> > chainSet; // same thing as chains but with unique chains
	QList<QList<int> > ownChains; // chains that ai will get in this run. those chains are taken in myLines.
	QList<int> ownMoves; // contains lines of chain that the ai will take first (this will contain the returned move)
	QScopedArrayPointer<bool> myLines(new bool[linesSize]); //make temporary local copies of lists
	int ownLinesCnt = 0; // count of how many lines ai will take in this run
	int ownSquaresCnt = 0; // count of how many squares ai will get in this run

	if (level > 1)
	{
		ownSquaresCnt = findOwnChains(lines, linesSize, width, height, &ownChains);
		kDebug() << "ownChains:" << ownChains;
    
    memcpy(myLines.data(), lines, linesSize);

		// complete the shortest chain first if there is more than one chain. this is needed to stop alternating between two chains because that works against the hard ai move which takes the next chain by sacrificing 2/4 squares. when alternating between two chains it's possible that there are 3 remaining open lines in both chains combined which triggers the evil move too late because the chains were completed in the wrong order
		int minChain=-1;
		int tmp=width*height*10;
		for(int i = 0; i < ownChains.size(); i++)
		{
      for (int j = 0; j < ownChains.at(i).size(); j++)
      {
        ownLinesCnt++;
        myLines[ownChains.at(i).at(j)] = true;
      }
			if(tmp > ownChains.at(i).size())
			{
				tmp = ownChains.at(i).size();
				minChain = i;
			}
		}
		if(minChain >= 0)
		{
			ownMoves=ownChains.at(minChain);
		}
		kDebug() << "ownMoves:" << ownMoves;
	}
	
	for(int i = 0; i < choiceList.size(); i++)	//cycle through all the possible moves
	{
		QList<int> squaresCopy = squareOwners;	//make temporary local copies of lists
		QSet<int> chain; // set of lines that are given to opponent by move choiceList.at(i)
		
		if (level > 1)
		{
			memcpy(linesCopy.data(), myLines.data(), linesSize);	//make temporary local copies of lists
			if (linesCopy[choiceList.at(i)]) continue; // already covered. ai will get this line
		} else {
			memcpy(linesCopy.data(), lines, linesSize);	//make temporary local copies of lists
		}
		
		linesCopy[choiceList.at(i)] = true;	//we're going to try drawing a line here
		
		//kDebug() << boardToString(linesCopy.data(), linesSize, width, height);
		
		//now it would be the next player's turn so we're going to count how many squares they would be able to get.
		int count = 0;	//this is how many points the next player will ge if you draw a line at choiceList.at(i)
		chain.insert(choiceList.at(i));
		QList<QList<int> > enemyChains;
		findOwnChains(linesCopy.data(), linesSize, width, height, &enemyChains);
		for (int j = 0; j < enemyChains.size(); j++)
		{
			//kDebug() << "enemyChains[" << j << "]: " << enemyChains.at(j);
			count += enemyChains.at(j).size();
			for (int k = 0; k < enemyChains.at(j).size(); k++)
			{
				chain.insert(enemyChains.at(j).at(k));
			}
		}
		//kDebug() << "lines: " << boardToString(lines);
		//kDebug() << "linesCopy: " << boardToString(linesCopy.data());
    //kDebug() << "chains in linesCopy: " << linelistToString(chain.toList());
		linePointDamage.insertMulti(count, choiceList.at(i));	//insert a pair with Key=count, Value=i
		chains.insert(choiceList.at(i), chain);
	}
	
	kDebug() << "linePointDamage:" << linePointDamage;
	
	if(level < 2) // middle ai won't analyze the game further
	{
		QList<int> bestMoves = linePointDamage.values(linePointDamage.begin().key());	//this is a list of the indices of the lines that are the least damaging. linePointDamage.begin() returns the 1st pair in the QMap, sorted in ascending order by Key (damage of move)
		return bestMoves;
	}

	//kDebug() << chains;
	// remove double entries from chains to get chainSet
	QMapIterator<int, QSet<int> > j(chains);
	while(j.hasNext()) // walk through chains and add chain to chainSet (if not already contained)
	{
		j.next();
		bool newChain = true;
		QSet<int> chainCheck = j.value(); // this is the chain we might add
		QMapIterator<int, QSet<int> > chainSetIt(chainSet);
		while(chainSetIt.hasNext()) // walk through chainSet and look for chainCheck
		{
			chainSetIt.next();
			QSet<int> chainSetI = chainSetIt.value();
			if(chainSetI == chainCheck) // found chainCheck in chainSet, don't add
			{
				newChain = false;
				break;
			}
		}
		if (newChain) // chainCheck not in chainSet
		{
			chainSet.insert(j.key(), chainCheck);
		}
	}
	kDebug() << "chainSet:" << chainSet;

	// analyze chains
	int shortChainCnt = 0; // chains <= 2 lines
	int longChainCnt = 0; // exploitable chains
	int loopChainCnt = 0; // also exploitable, but more costly
	QList<QSet<int> > shortChains;
  QList<QSet<int> > longChains;
  QList<QSet<int> > loopChains;
	QMapIterator<int, QSet<int> > chainSetIt(chainSet);
	while(chainSetIt.hasNext())
	{
		chainSetIt.next();
		QSet<int> chainSetI = chainSetIt.value();
    int classification = classifyChain(chainSetI.toList(), lines);
    kDebug() << "analysing chain " << chainSetI << ": " << classification;
		switch (classification)
		{
			case KSquares::CHAIN_LONG: 
        longChainCnt++;
        longChains.append(chainSetI);
      break;
			case KSquares::CHAIN_SHORT:
        shortChainCnt++;
        shortChains.append(chainSetI);
      break;
			case KSquares::CHAIN_LOOP:
        loopChainCnt++;
        loopChains.append(chainSetI);
      break;
      default:
        kDebug() << "unknown chain type " << classification;
    }
	}
	kDebug() << "short chains:" << shortChainCnt << ", long chains: " << longChainCnt << ", loop chains: " << loopChainCnt << ", ownLinesCnt: " << ownLinesCnt << ", ownSquaresCnt: " << ownSquaresCnt;

	if(
		(
		    (ownLinesCnt == 2) || // sacrifice 2 squares squares to opponent to get next chain. 
		    (ownLinesCnt == 3 && ownSquaresCnt == 4) // this is for loop chains which require a sacrifice of 4 squares
		) 
		&&
		longChainCnt + loopChainCnt > 0 // only do it if there is at least one chain to steal
		&& 
		shortChainCnt == 0 // if there is another short chain just take the own chain and do a hard hearted handout in the chain left
		&&
		safeMoves(linesSize, lines).size() == 0 // only do it in endgames
	  )
	{
		kDebug() << "HAHA, our chance to do the evil thing!";
		int magicLine = -1; // line in own moves that is used to get the next chain (draw there to give 2/4 squares to opponent)
		// formal definition of magicLine: line that when completed will leave at least one other line in own moves that completes two squares at once
		// the opposite of magic line will be used in the hard hearted handout to make sure that the opponent won't be able to do the evil move
		for(int i = 0; i < ownMoves.size(); i++)
		{
			memcpy(myLines.data(), lines, linesSize); // we want to look one line into the future game
			myLines[ownMoves.at(i)] = true; // try ownMove i (one line in chain that ai will get)
			for(int j = 0; j < ownMoves.size(); j++) // test other lines in own moves
			{
				if (i == j) continue;
				int leftSquares = 0; // count squares that can be completed by other line (j)
				QList<int> adjacentSquares = squaresFromLine(ownMoves.at(j));
				for(int k = 0; k < adjacentSquares.size(); k++)
				{
					if (countBorderLines(adjacentSquares.at(k), &(*myLines)) == 3)
					{
						leftSquares++;
					}
				}
				if (leftSquares == 2) // we found a line that will yield another line in own moves that completes two squares
				{
					magicLine = i;
				}
			}
		}
		kDebug() << "Magic Line index:" << magicLine;
		QList<int> bestMoves;
		if(ownMoves.size() > 1)
		{
			int ownMove = 1;
			if(magicLine >= 0 && magicLine < ownMoves.size())
			{
				ownMove=magicLine;
			}
			bestMoves.append(ownMoves.at(ownMove)); // choose the second line found! in case of 2 squares for ai this will choose the line at the end of the chain. in case of 4 squares this will be the line in the middle, leaving two lines that complete two squares each. FIX: 1 doesn't work in some cases because the algorithm doesn't detect chains by spatial connectedness. ie if there are two ends of a chain the search algorithm can jump between those two ends, messing up the order in ownMoves list. solution is magicLine
			return bestMoves;
		}
	}

	if(ownMoves.size() > 0) // complete own chain
	{
		QList<int> bestMoves;
		bestMoves.append(ownMoves.at(0));
		return bestMoves;
	}

	if(linePointDamage.begin().key() == 2) // opponent will get 2 squares
	{
		int handoutLine = -1;
		QList<int> opponentChain;
		QMapIterator<int, QSet<int> > chainSetIt(chainSet);
		while(chainSetIt.hasNext())
		{
			chainSetIt.next();
			QSet<int> chainSetI = chainSetIt.value();
			if(chainSetI.contains(linePointDamage.begin().value()))
			{
				opponentChain = chainSetI.values();
			}
		}
		for(int i = 0; i < opponentChain.size(); i++)
		{
			memcpy(myLines.data(), lines, linesSize); // we want to look one line into the future game
			myLines[opponentChain.at(i)] = true; // try move in chain for opponent
			for(int j = 0; j < opponentChain.size(); j++) // test other lines in chain
			{
				if (i == j) continue;
				int badSquares = 0; // count squares with two open lines (those are dangerous)
				QList<int> adjacentSquares = squaresFromLine(opponentChain.at(j));
				for(int k = 0; k < adjacentSquares.size(); k++)
				{
					if(countBorderLines(adjacentSquares.at(k), &(*myLines)) != 3)
					{
						badSquares++;
					}
				}
				if(badSquares == 0)
				{
					handoutLine = i;
				}
			}
		}
		if(handoutLine >= 0)
		{
			kDebug() << "Hard hearted handout at" << opponentChain.at(handoutLine);
			QList<int> retMove;
			retMove.append(opponentChain.at(handoutLine));
			return retMove;
		}
	}

	// fallback to middle ai move
	QList<int> bestMoves = linePointDamage.values(linePointDamage.begin().key());	//this is a list of the indices of the lines that are the least damaging. linePointDamage.begin() returns the 1st pair in the QMap, sorted in ascending order by Key (damage of move)
	return bestMoves;
}
