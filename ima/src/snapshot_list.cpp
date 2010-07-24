/*
 * snapshot_list.cpp - implementation of snapshot-list for side-bar
 *
 * Copyright (c) 2004-2010 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
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

#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtGui/QScrollArea>


#include "snapshot_list.h"
#include "client.h"
#include "local_system.h"



snapshotList::snapshotList( MainWindow * _main_window, QWidget * _parent ) :
	SideBarWidget( QPixmap( ":/resources/snapshot.png" ),
			tr( "Snapshots" ),
			tr( "Simply manage the snapshots you made using this "
				"workspace." ),
			_main_window, _parent )
{
	setupUi( contentParent() );

	connect( list, SIGNAL( currentTextChanged( const QString & ) ), this,
				SLOT( snapshotSelected( const QString & ) ) );
	connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this,
			SLOT( snapshotActivated( QListWidgetItem * ) ) );

	previewLbl->setScaledContents( TRUE );

	QFont f = userDescLbl->font();
	f.setBold( TRUE );
	f.setItalic( TRUE );

	userDescLbl->setFont( f );
	dateDescLbl->setFont( f );
	timeDescLbl->setFont( f );
	hostDescLbl->setFont( f );

	f.setBold( FALSE );
	f.setItalic( FALSE );

	connect( showBtn, SIGNAL( clicked() ), this,
						SLOT( showSnapshot() ) );

	connect( deleteBtn, SIGNAL( clicked() ), this,
						SLOT( deleteSnapshot() ) );
	connect( reloadBtn, SIGNAL( clicked() ), this, SLOT( reloadList() ) );

	reloadList();
}




snapshotList::~snapshotList()
{
}





void snapshotList::snapshotSelected( const QString & _s )
{
	previewLbl->setPixmap( localSystem::snapshotDir() + _s );
	previewLbl->setFixedHeight( previewLbl->width() * 3 / 4 );
	userLbl->setText( _s.section( '_', 0, 0 ) );
 	hostLbl->setText( _s.section( '_', 1, 1 ) );
	dateLbl->setText( QDate::fromString( _s.section( '_', 2, 2 ),
				Qt::ISODate ).toString( Qt::LocalDate ) );
	timeLbl->setText( _s.section( '_', 3, 3 ).section( '.', 0, 0 ).
							replace( '-', ':' ) );
}




void snapshotList::snapshotDoubleClicked( const QString & _s )
{
	// maybe the user clicked on "show snapshot" and selected no
	// snapshot...
	if( _s == "" )
	{
		return;
	}

	QLabel * img_label = new QLabel;
	img_label->setPixmap( localSystem::snapshotDir() + _s );
	if( img_label->pixmap() != NULL )
	{
		img_label->setFixedSize( img_label->pixmap()->width(),
					img_label->pixmap()->height() );
	}

	QScrollArea * sa = new QScrollArea;
	sa->setAttribute( Qt::WA_DeleteOnClose, TRUE );
	sa->move( 0, 0 );
	sa->setWidget( img_label );
	sa->setWindowTitle( _s );
	sa->show();
}




void snapshotList::showSnapshot( void )
{
	if( list->currentItem() )
	{
		snapshotDoubleClicked( list->currentItem()->text() );
	}
}




void snapshotList::deleteSnapshot( void )
{
	if( !list->currentItem() )
	{
		return;
	}

	const QString s = list->currentItem()->text();

	// maybe the user clicked on "delete snapshot" and didn't select a
	// snapshot...
	if( s.isEmpty() )
	{
		return;
	}

	QFile( localSystem::snapshotDir() + s ).remove();

	reloadList();
}




void snapshotList::reloadList( void )
{
	QDir sdir( localSystem::snapshotDir(), "*.png",
						QDir::Name | QDir::IgnoreCase,
						QDir::Files | QDir::Readable );

	list->clear();
	list->insertItems( 0, sdir.entryList() );
}



#include "snapshot_list.moc"

