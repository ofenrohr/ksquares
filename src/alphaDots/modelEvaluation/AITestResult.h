//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_AITESTRESULT_H
#define KSQUARES_AITESTRESULT_H


#include <QtCore/QVariant>
#include "AITestSetup.h"

class AITestResult {
public:
	AITestSetup setup;
	QString nameP1;
	QString nameP2;
	QList<int> moves;
	QList<int> timeP1;
	QList<int> timeP2;
	bool taintedP1;
	bool taintedP2;
	int crashesP1;
	int crashesP2;
	int scoreP1;
	int scoreP2;

	QVariant toQVariant();
	void fromQVariant(QVariant map);
};


#endif //KSQUARES_AITESTRESULT_H
