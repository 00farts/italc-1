/*
 * main.cpp - main file for iTALC Management Console
 *
 * Copyright (c) 2010 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
 *
 * This file is part of iTALC - http://italc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <italcconfig.h>

#include <QtCore/QLocale>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>

#include "Configuration/XmlStore.h"
#include "ImcCore.h"
#include "ItalcConfiguration.h"
#include "ItalcCore.h"
#include "MainWindow.h"
#include "LocalSystem.h"
#include "Logger.h"



int main( int argc, char **argv )
{
	// make sure to run as admin
	if( !LocalSystem::Process::isRunningAsAdmin() )
	{
		QCoreApplication app( argc, argv );
		QStringList args = app.arguments();
		args.removeFirst();
		LocalSystem::Process::runAsAdmin(
				QCoreApplication::applicationFilePath(),
				args.join( " " ) );
		return 0;
	}

	QCoreApplication *app = NULL;
#ifdef ITALC_BUILD_LINUX
	if( QProcessEnvironment::systemEnvironment().contains( "DISPLAY" ) )
	{
		app = new QApplication( argc, argv );
	}
	else
	{
		app = new QCoreApplication( argc, argv );
	}
#else
	app = new QApplication( argc, argv );
#endif

	ItalcCore::init();

	Logger l( "ItalcManagementConsole" );

	if( !ItalcConfiguration().isStoreWritable() &&
			ItalcCore::config->logLevel() < Logger::LogLevelDebug )
	{
		ImcCore::criticalMessage( MainWindow::tr( "Configuration not writable" ),
			MainWindow::tr( "The local configuration backend reported that the "
							"configuration is not writable! Please run the iTALC "
							"Management Console with higher privileges." ) );
		return -1;
	}

	app->connect( app, SIGNAL( lastWindowClosed() ), SLOT( quit() ) );

	const QString loc = QLocale::system().name().left( 2 );

	foreach( const QString & qm, QStringList()
												<< loc + "-core"
												<< loc
												<< "qt_" + loc )
	{
		QTranslator * tr = new QTranslator( app );
		tr->load( QString( ":/resources/%1.qm" ).arg( qm ) );
		app->installTranslator( tr );
	}

	// parse arguments
	QStringListIterator argIt( QCoreApplication::arguments() );
	argIt.next();

	while( argc > 1 && argIt.hasNext() )
	{
		const QString a = argIt.next().toLower();
		if( ( a == "-applysettings" || a == "-a" ) && argIt.hasNext() )
		{
			const QString file = argIt.next();
			Configuration::XmlStore xs( Configuration::XmlStore::System, file );

			if( ImcCore::applyConfiguration( ItalcConfiguration( &xs ) ) )
			{
				ImcCore::informationMessage(
					MainWindow::tr( "iTALC Management Console" ),
					MainWindow::tr( "All settings were applied successfully." ) );
			}
			else
			{
				ImcCore::criticalMessage(
					MainWindow::tr( "iTALC Management Console" ),
					MainWindow::tr( "An error occured while applying settings!" ) );
			}

			// remove temporary file
			QFile( file ).remove();

			return 0;
		}
		else if( a == "-listconfig" || a == "-l" )
		{
			ImcCore::listConfiguration( *ItalcCore::config );

			return 0;
		}
		else if( a == "-setconfigvalue" || a == "-s" )
		{
			if( !argIt.hasNext() )
			{
				qCritical( "No configuration property specified!" );
				return -1;
			}
			QString prop = argIt.next();
			QString value;
			if( !argIt.hasNext() )
			{
				if( !prop.contains( '=' ) )
				{
					qCritical() << "No value for property" << prop << "specified!";
					return -1;
				}
				else
				{
					value = prop.section( '=', -1, -1 );
					prop = prop.section( '=', 0, -2 );
				}
			}
			else
			{
				value = argIt.next();
			}
			const QString key = prop.section( '/', -1, -1 );
			const QString parentKey = prop.section( '/', 0, -2 );

			ItalcCore::config->setValue( key, value, parentKey );

			ImcCore::applyConfiguration( *ItalcCore::config );

			return 0;
		}
		else if( a == "-importpublickey" || a == "-i" )
		{
			QString pubKeyFile;
			if( !argIt.hasNext() )
			{
				QStringList l =
					QDir::current().entryList( QStringList() << "*.key.txt",
												QDir::Files | QDir::Readable );
				if( l.size() != 1 )
				{
					qCritical( "Please specify location of the public key "
								"to import" );
					return -1;
				}
				pubKeyFile = QDir::currentPath() + QDir::separator() +
													l.first();
				qWarning() << "No public key file specified. Trying to import "
								"the public key file found at" << pubKeyFile;
			}
			else
			{
				pubKeyFile = argIt.next();
			}

			if( !ImcCore::importPublicKey( ItalcCore::RoleTeacher,
											pubKeyFile, QString() ) )
			{
				LogStream( Logger::LogLevelInfo ) << "Public key import "
													"failed";
				return -1;
			}
			LogStream( Logger::LogLevelInfo ) << "Public key successfully "
													"imported";
			return 0;
		}
	}

	// now create the main window
	MainWindow *mainWindow = new MainWindow;

	mainWindow->show();

	ilog( Info, "App.Exec" );

	int ret = app->exec();

	ItalcCore::destroy();

	delete app;

	return ret;
}

