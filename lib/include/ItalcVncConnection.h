/*
 * ItalcVncConnection.h - declaration of ItalcVncConnection class
 *
 * Copyright (c) 2008-2010 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
 *
 * This file is part of iTALC - http://italc.sourceforge.net
 *
 * code partly taken from KRDC / vncclientthread.h:
 * Copyright (C) 2007-2008 Urs Wolfer <uwolfer @ kde.org>
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

#ifndef _ITALC_VNC_CONNECTION_H
#define _ITALC_VNC_CONNECTION_H

#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QReadWriteLock>
#include <QtCore/QThread>

#include "ItalcCore.h"
#include "FastQImage.h"

extern "C"
{
	#include <rfb/rfbclient.h>
}


class ClientEvent
{
public:
	virtual ~ClientEvent()
	{
	}

	virtual void fire( rfbClient *c ) = 0;
} ;


class ItalcVncConnection: public QThread
{
	Q_OBJECT
public:
	enum QualityLevels
	{
		ThumbnailQuality,
		SnapshotQuality,
		RemoteControlQuality,
		DemoQuality,
		NumQualityLevels
	} ;

	explicit ItalcVncConnection( QObject *parent = 0 );
	virtual ~ItalcVncConnection();

	const QImage image( int x = 0, int y = 0, int w = 0, int h = 0 );
	void setImage( const QImage &img );
	void emitUpdated( int x, int y, int w, int h );
	void emitCursorShapeUpdated( const QImage &cursorShape, int xh, int yh );
	void emitGotCut( const QString &text );
	void stop();
	void reset( const QString &host );
	void setHost( const QString &host );
	void setPort( int port );

	bool isConnected() const
	{
		return m_connected && isRunning();
	}

	void setItalcAuthType( ItalcAuthType t )
	{
		m_italcAuthType = t;
	}

	ItalcAuthType italcAuthType() const
	{
		return m_italcAuthType;
	}

	void setQuality( QualityLevels qualityLevel )
	{
		m_quality = qualityLevel;
	}

	QualityLevels quality() const
	{
		return m_quality;
	}

	void enqueueEvent( ClientEvent *e );

	rfbClient * getRfbClient()
	{
		return m_cl;
	}

	QSize framebufferSize() const
	{
		return m_image.size();
	}

	void setScaledSize( const QSize &s )
	{
		if( m_scaledSize != s )
		{
			m_scaledSize = s;
			m_scaledScreenNeedsUpdate = true;
		}
	}

	FastQImage scaledScreen()
	{
		rescaleScreen();
		return m_scaledScreen;
	}

	void setFramebufferUpdateInterval( int interval );

	void rescaleScreen();


	uint8_t * frameBuffer;


signals:
	void newClient( rfbClient *c );
	void imageUpdated( int x, int y, int w, int h );
	void cursorShapeUpdated( const QImage &cursorShape, int xh, int yh );
	void gotCut( const QString &text );
	void passwordRequest();
	void outputErrorMessage( const QString &message );
	void connected();


public slots:
	void mouseEvent( int x, int y, int buttonMask );
	void keyEvent( int key, bool pressed );
	void clientCut( const QString &text );


protected:
	virtual void run();


private:
	// hooks for LibVNCClient
	static rfbBool hookNewClient( rfbClient *cl );
	static void hookUpdateFB( rfbClient *cl, int x, int y, int w, int h );
	static void hookCursorShape( rfbClient *cl, int xh, int yh, int w, int h, int bpp );
	static void hookCutText( rfbClient *cl, const char *text, int textlen );
	static void hookOutputHandler( const char *format, ... );
	static rfbBool hookHandleItalcMessage( rfbClient *cl,
						rfbServerToClientMsg *msg );

	rfbClient *m_cl;
	ItalcAuthType m_italcAuthType;
	QualityLevels m_quality;
	QString m_host;
	int m_port;
	int m_framebufferUpdateInterval;
	QMutex m_mutex;
	QReadWriteLock m_imgLock;
	QQueue<ClientEvent *> m_eventQueue;

	FastQImage m_image;
	bool m_scaledScreenNeedsUpdate;
	FastQImage m_scaledScreen;
	QSize m_scaledSize;

	volatile bool m_connected;
	volatile bool m_stopped;


private slots:
	void checkOutputErrorMessage();

} ;

#endif

