/*
 * system_key_trapper.cpp - class for trapping system-keys and -key-sequences
 *                          such as Alt+Ctrl+Del, Alt+Tab etc.
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


#define XK_KOREAN
#include "rfb/keysym.h"
#include "system_key_trapper.h"

#include <QtCore/QList>
#include <QtCore/QTimer>

static QMutex __trapped_keys_mutex;
static QList<systemKeyTrapper::trappedKeys> __trapped_keys;

QMutex systemKeyTrapper::s_refCntMutex;
int systemKeyTrapper::s_refCnt;


#if BUILD_WIN32

// some code for disabling system's hotkeys such as Alt+Ctrl+Del, Alt+Tab,
// Ctrl+Esc, Alt+Esc, Windows-key etc. - otherwise locking wouldn't be very
// effective... ;-)
#define _WIN32_WINNT 0x0500 // for KBDLLHOOKSTRUCT

#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winreg.h>
#include <winuser.h>

#include "inject.h"



HHOOK g_hHookKbdLL = NULL; // hook handle


LRESULT CALLBACK TaskKeyHookLL( int nCode, WPARAM wp, LPARAM lp )
{
	KBDLLHOOKSTRUCT *pkh = (KBDLLHOOKSTRUCT *) lp;
	static QList<systemKeyTrapper::trappedKeys> pressed;
	if( nCode == HC_ACTION )
	{
		BOOL bCtrlKeyDown = GetAsyncKeyState( VK_CONTROL ) >>
						( ( sizeof( SHORT ) * 8 ) - 1 );
		QMutexLocker m( &__trapped_keys_mutex );

		systemKeyTrapper::trappedKeys key = systemKeyTrapper::None;
		if( pkh->vkCode == VK_ESCAPE && bCtrlKeyDown )
		{
			key = systemKeyTrapper::CtrlEsc;
		}
		else if( pkh->vkCode == VK_TAB && pkh->flags & LLKHF_ALTDOWN )
		{
			key = systemKeyTrapper::AltTab;
		}
		else if( pkh->vkCode == VK_ESCAPE &&
						pkh->flags & LLKHF_ALTDOWN )
		{
			key = systemKeyTrapper::AltEsc;
		}
		else if( pkh->vkCode == VK_SPACE && pkh->flags & LLKHF_ALTDOWN )
		{
			key = systemKeyTrapper::AltSpace;
		}
		else if( pkh->vkCode == VK_F4 && pkh->flags & LLKHF_ALTDOWN )
		{
			key = systemKeyTrapper::AltF4;
		}
		else if( pkh->vkCode == VK_LWIN || pkh->vkCode == VK_RWIN )
		{
			key = systemKeyTrapper::MetaKey;
		}
		else if( pkh->vkCode == VK_DELETE && bCtrlKeyDown &&
						pkh->flags && LLKHF_ALTDOWN )
		{
			key = systemKeyTrapper::AltCtrlDel;
		}
		if( key != systemKeyTrapper::None )
		{
			if( !pressed.contains( key ) )
			{
				__trapped_keys.push_back( key );
				pressed << key;
			}
			else
			{
				pressed.removeAll( key );
			}
			return( 1 );
		}
	}
	return CallNextHookEx( g_hHookKbdLL, nCode, wp, lp );
}




static STICKYKEYS settings_sk = { sizeof( STICKYKEYS ), 0 };
static TOGGLEKEYS settings_tk = { sizeof( TOGGLEKEYS ), 0 };
static FILTERKEYS settings_fk = { sizeof( FILTERKEYS ), 0 };


void enableStickyKeys( bool _enable )
{
	if( _enable )
	{
		STICKYKEYS sk = settings_sk;
		TOGGLEKEYS tk = settings_tk;
		FILTERKEYS fk = settings_fk;

		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ),
							&settings_sk, 0 );
		SystemParametersInfo( SPI_SETTOGGLEKEYS, sizeof( TOGGLEKEYS ),
							&settings_tk, 0 );
		SystemParametersInfo( SPI_SETFILTERKEYS, sizeof( FILTERKEYS ),
							&settings_fk, 0 );
	}
	else
	{
		SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ),
							&settings_sk, 0 );
		SystemParametersInfo( SPI_GETTOGGLEKEYS, sizeof( TOGGLEKEYS ),
							&settings_tk, 0 );
		SystemParametersInfo( SPI_GETFILTERKEYS, sizeof( FILTERKEYS ),
							&settings_fk, 0 );

		STICKYKEYS skOff = settings_sk;
		skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
		skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ),
								&skOff, 0 );

		TOGGLEKEYS tkOff = settings_tk;
		tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
		tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;
		SystemParametersInfo( SPI_SETTOGGLEKEYS, sizeof( TOGGLEKEYS ),
								&tkOff, 0 );

		FILTERKEYS fkOff = settings_fk;
		fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
		fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;
		SystemParametersInfo( SPI_SETFILTERKEYS, sizeof( FILTERKEYS ),
								&fkOff, 0 );
	}
}


#endif


systemKeyTrapper::systemKeyTrapper( bool _enabled ) :
	QObject(),
	m_enabled( FALSE )
{
	setEnabled( _enabled );
}




systemKeyTrapper::~systemKeyTrapper()
{
	setEnabled( FALSE );
}




void systemKeyTrapper::setEnabled( bool _on )
{
	if( _on == m_enabled )
	{
		return;
	}

	s_refCntMutex.lock();
	m_enabled = _on;
	if( _on )
	{
#if BUILD_WIN32
		if( !s_refCnt )
		{
			if( !g_hHookKbdLL )
			{
				HINSTANCE hAppInstance =
							GetModuleHandle( NULL );
				// set lowlevel-keyboard-hook
				g_hHookKbdLL =
					SetWindowsHookEx( WH_KEYBOARD_LL,
								TaskKeyHookLL,
								hAppInstance,
								0 );
			}

			enableStickyKeys( FALSE );

			//EnableWindow( FindWindow( "Shell_traywnd", NULL ),
			//						FALSE );

			Inject();
		}

		QTimer * t = new QTimer( this );
		connect( t, SIGNAL( timeout() ),
					this, SLOT( checkForTrappedKeys() ) );
		t->start( 10 );
#endif
		++s_refCnt;
	}
	else
	{
		--s_refCnt;
#if BUILD_WIN32
		if( !s_refCnt )
		{
			UnhookWindowsHookEx( g_hHookKbdLL );
			g_hHookKbdLL = NULL;

			enableStickyKeys( TRUE );
			//EnableWindow( FindWindow( "Shell_traywnd", NULL ),
			//						TRUE );

			Eject();
		}
#endif
	}
	s_refCntMutex.unlock();
}




void systemKeyTrapper::checkForTrappedKeys( void )
{
	QMutexLocker m( &__trapped_keys_mutex );

	while( !__trapped_keys.isEmpty() )
	{
		int key = 0;
		switch( __trapped_keys.front() )
		{
			case None: break;
			case AltCtrlDel: key = XK_Delete; break;
			case AltTab: key = XK_Tab; break;
			case AltEsc: key = XK_Escape; break;
			case AltSpace: key = XK_KP_Space; break;
			case AltF4: key = XK_F4; break;
			case CtrlEsc: key = XK_Escape; break;
			case MetaKey: key = XK_Meta_L; break;
		}

		if( key )
		{
			emit keyEvent( key, TRUE );
			emit keyEvent( key, FALSE );
		}

		__trapped_keys.removeFirst();

	}
}


#include "system_key_trapper.moc"

