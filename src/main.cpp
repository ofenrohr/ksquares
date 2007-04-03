/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <khighscore.h>
#include <kexthighscore.h>

#include <kdebug.h>

#include "ksquareswindow.h"
#include "ksquaresdemowindow.h"

static const KCmdLineOptions options[] =
{
	{ "demo", I18N_NOOP("Run game in demo (autoplay) mode"), 0 },
	KCmdLineLastOption // End of options.
};

static const char description[] =
    I18N_NOOP("Take it in turns to draw lines.\nIf you complete a squares, you get another go.");

static const char version[] = "0.3";

int main(int argc, char **argv)
{
	KAboutData about("ksquares", I18N_NOOP("KSquares"), version, description,
			 KAboutData::License_GPL, "(C) 2006-2007 Matt Williams", 0, 
			 "http://milliams.com/content/view/18/42/", "matt@milliams.com");
	about.addAuthor( "Matt Williams", I18N_NOOP("Original creator and maintainer"), "matt@milliams.com", "http://milliams.com" );
	about.addCredit("Fela Winkelmolen", I18N_NOOP("Many patches and bugfixes"));
	
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	
	KApplication app;
	
	if (args->isSet("demo"))
	{
		kDebug() << "Running in demo mode. Not implememnted yet..." << endl;
		KSquaresDemoWindow *demoWindow = new KSquaresDemoWindow;
		demoWindow->show();
		demoWindow->gameNew();
	}
	else
	{
		KHighscore::init("ksquares");
		KExtHighscore::Manager highscoresManager;
		
		KSquaresWindow *mainWindow = new KSquaresWindow;
		mainWindow->show();
		mainWindow->gameNew();
	}
	args->clear();
	
	return app.exec();
}
