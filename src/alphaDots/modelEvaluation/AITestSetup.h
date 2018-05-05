//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_AITESTSETUP_H
#define KSQUARES_AITESTSETUP_H


#include <QtCore/QVariant>
#include <QtCore/QPoint>
#include <aicontroller.h>

class AITestSetup {
public:
	/// Used in
	/// 0-2 = Easy, Medium, Hard, 3-n = evaluated ai+model combination
	int aiLevelP1;
	int aiLevelP2; // 0-2 = Easy, Medium, Hard, 3-n = evaluated ai+model combination
	int timeout;
	QPoint boardSize;
	QString modelNameP1; // name of neural network to use
	QString modelNameP2; // name of neural network to use
	KSquares::AILevel modelAiP1;
	KSquares::AILevel modelAiP2;

	QVariant toQVariant();
	void fromQVariant(QVariant map);
};


#endif //KSQUARES_AITESTSETUP_H
