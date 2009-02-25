/*
 * local_system.h - misc. platform-specific stuff
 *
 * Copyright (c) 2006-2007 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
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


#ifndef _LOCAL_SYSTEM_H
#define _LOCAL_SYSTEM_H

#include <QtCore/QString>

#include "isd_base.h"

#ifdef BUILD_WIN32
#include <windef.h>
#endif

class QWidget;

extern __declspec(dllimport) QByteArray __appInternalChallenge;


namespace localSystem
{
	extern int __declspec(dllimport) logLevel;

	typedef void (*p_pressKey)( int _key, bool _down );


	int freePort( void );

	void initialize( p_pressKey _pk, const QString & _log_file );

	void sleep( const int _ms );

	void execInTerminal( const QString & _cmds );

	void broadcastWOLPacket( const QString & _mac );

	void powerDown( void );
	void reboot( void );

	void logonUser( const QString & _uname, const QString & _pw,
						const QString & _domain );
	void logoutUser( void );

	QString currentUser( void );

	QString privateKeyPath( const ISD::userRoles _role,
						bool _only_path = FALSE );
	QString publicKeyPath( const ISD::userRoles _role,
						bool _only_path = FALSE );

	void setPrivateKeyPath( const QString & _path,
						const ISD::userRoles _role );
	void setPublicKeyPath( const QString & _path,
						const ISD::userRoles _role );

	QString snapshotDir( void );
	QString globalConfigPath( void );
	QString personalConfigDir( void );
	QString personalConfigPath( void );

	QString globalStartmenuDir( void );

	QString parameter( const QString & _name );

	bool ensurePathExists( const QString & _path );

	QString ip( void );

	QString userRoleName( const ISD::userRoles _role );

#ifdef BUILD_WIN32
	BOOL enablePrivilege( LPCTSTR lpszPrivilegeName, BOOL bEnable );
#endif

	void activateWindow( QWidget * _window );

}


#endif
