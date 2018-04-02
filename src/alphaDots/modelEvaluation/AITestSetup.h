//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_AITESTSETUP_H
#define KSQUARES_AITESTSETUP_H


#include <QtCore/QVariant>
#include <QtCore/QPoint>

class AITestSetup {
public:
	int levelP1;
	int levelP2;
	int timeout;
	QPoint boardSize;
	QString modelNameP1;
	QString modelNameP2;

	QVariant toQVariant();
	void fromQVariant(QVariant map);
};


#endif //KSQUARES_AITESTSETUP_H
