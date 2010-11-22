/*
 * AuthenticationCredentials.h - class holding credentials for authentication
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

#ifndef _AUTHENTICATION_CREDENTIALS_H
#define _AUTHENTICATION_CREDENTIALS_H

#include <QtCore/QString>

class PrivateDSAKey;

class AuthenticationCredentials
{
public:
	enum TypeFlags
	{
		PrivateKey = 0x01,
		UserLogon = 0x02,
		NumTypes
	} ;
	typedef int TypeFlag;

	AuthenticationCredentials();

	bool hasCredentials( TypeFlags credentialType ) const;

	bool loadPrivateKey( const QString &path );
	const PrivateDSAKey *privateKey() const
	{
		return m_privateKey;
	}

	void setLogonCredentials( const QString &username, const QString &password );

	const QString &logonUsername() const
	{
		return m_logonUsername;
	}

	const QString &logonPassword() const
	{
		return m_logonPassword;
	}

private:
	PrivateDSAKey *m_privateKey;
	QString m_logonUsername;
	QString m_logonPassword;

} ;
#endif
